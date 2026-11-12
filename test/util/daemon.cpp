// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <array>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>
#include <future>
#include <gtest/gtest.h>
#include <limits>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/testing/syscall/_exit.hpp>
#include <mcs/testing/syscall/eventfd.hpp>
#include <mcs/testing/syscall/kill.hpp>
#include <mcs/testing/syscall/sigaction.hpp>
#include <mcs/testing/syscall/sigemptyset.hpp>
#include <mcs/testing/syscall/waitpid.hpp>
#include <mcs/util/FMT/write_file.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/daemon.hpp>
#include <mcs/util/read_file.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/dup.hpp>
#include <mcs/util/syscall/dup2.hpp>
#include <mcs/util/syscall/fflush.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/fork.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <mcs/util/syscall/pipe.hpp>
#include <mcs/util/syscall/read.hpp>
#include <mcs/util/syscall/write.hpp>
#include <mcs/util/syscall/write_retrying_on_interrupt.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <vector>

namespace mcs::util
{
  namespace
  {
    auto test_help (arguments::View args) -> void
    {
      fmt::print
        ( "Usage: {} [--mcs-daemon-foreground]\n"
          "Run the daemon lifecycle test implementation.\n"
        , args.at (0)
        );
    }

    struct ConvertibleToInt
    {
      operator int() const;
    };

    static_assert
      ( is_daemonized_main_implementation
          < int (*) (arguments::View, Daemon&)
          >
      );
    static_assert
      ( !is_daemonized_main_implementation
          < short (*) (arguments::View, Daemon&)
          >
      );
    static_assert
      ( !is_daemonized_main_implementation
          < bool (*) (arguments::View, Daemon&)
          >
      );
    static_assert
      ( !is_daemonized_main_implementation
           < ConvertibleToInt (*) (arguments::View, Daemon&)
           >
      );

    volatile std::sig_atomic_t signal_was_handled {0};

    auto handle_signal (int) noexcept -> void
    {
      signal_was_handled = 1;
    }

    auto process_exists (pid_t pid) -> bool
    {
      return std::filesystem::exists
        ( std::filesystem::path {"/proc"} / std::to_string (pid)
        );
    }

    auto read_pid (std::filesystem::path const& path) -> pid_t
    {
      auto const pid_string {read_file (path)};
      auto pid {pid_t {0}};
      if (std::from_chars ( pid_string.data()
                          , pid_string.data() + pid_string.size()
                          , pid
                          ).ec == std::errc{}
         )
      {
        return pid;
      }
      throw std::runtime_error {"read_pid failed"};
    }

    auto wait_for_exit (pid_t pid) -> int
    {
      auto status {int {0}};
      EXPECT_EQ
        ( testing::syscall::waitpid (pid, std::addressof (status), 0)
        , pid
        );
      EXPECT_TRUE (WIFEXITED (status));
      return WEXITSTATUS (status);
    }

    auto read_event_counter (int fd) -> std::uint64_t
    {
      auto value {std::uint64_t {0}};
      auto const bytes_read
        { syscall::read (fd, std::addressof (value), sizeof (value))
        };

      EXPECT_EQ (bytes_read, sizeof (value));

      syscall::close (fd);

      return value;
    }

    auto write_event_counter (int fd, std::uint64_t value) -> void
    {
      auto const bytes_written
        { syscall::write (fd, std::addressof (value), sizeof (value))
        };

      EXPECT_EQ (bytes_written, sizeof (value));

      syscall::close (fd);
    }

    auto stop_daemon_and_expect_exit (pid_t daemon_pid) -> void
    {
      for (auto n {0}; n < 200 && process_exists (daemon_pid); ++n)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds {10});
      }

      if (process_exists (daemon_pid))
      {
        testing::syscall::kill (daemon_pid, SIGTERM);
      }

      EXPECT_FALSE (process_exists (daemon_pid));
    }

    auto read_until_eof (int fd) -> std::string
    {
      auto buffer {std::array<char, 1 << 10>{}};

      auto const read_chunk
        { [&]
          {
            return std::span<char>
              { buffer.data()
              , buffer.data() + syscall::read (fd, buffer.data(), buffer.size())
              };
          }
        };

      auto content {std::string{}};

      for (auto chunk {read_chunk()}; !chunk.empty(); chunk = read_chunk())
      {
        content.append (chunk.data(), chunk.size());
      }

      syscall::close (fd);

      return content;
    }
  }

  TEST (MCSUtilDaemon, foreground_option_is_filtered_and_does_not_fork)
  {
    auto const caller_pid {syscall::getpid()};
    auto implementation_pid {pid_t {0}};
    auto implementation_arguments {std::vector<std::string>{}};
    auto const argv
      { std::array<char const*, 6>
          { "daemonize_test"
          , "--mcs-daemon-foreground"
          , "EXPECTED_ARGUMENT"
          , "ANOTHER_ARGUMENT"
          , "--mcs-daemon-foreground"
          , "FINAL_ARGUMENT"
          }
      };

    auto const result
      { daemonize
        ( arguments::View {argv.data(), argv.size()}
        , [&] (arguments::View args, Daemon& daemon) -> int
          {
            implementation_pid = syscall::getpid();

            for (auto const* argument : args)
            {
              implementation_arguments.emplace_back (argument);
            }

            daemon.ready();

            return EXIT_SUCCESS;
          }
        )
      };
    auto const expected_arguments
      { std::vector<std::string>
          { "daemonize_test"
          , "EXPECTED_ARGUMENT"
          , "ANOTHER_ARGUMENT"
          , "FINAL_ARGUMENT"
          }
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
    EXPECT_EQ (implementation_pid, caller_pid);
    EXPECT_EQ (implementation_arguments, expected_arguments);
  }

  TEST (MCSUtilDaemon, embedded_foreground_flags_are_not_consumed)
  {
    auto const pointers
      { std::array
        { "program", "[[FUSE", "--mcs-daemon-foreground"
        , "[[", "--mcs-daemon-foreground", "]]"
        , "[[BEE", "--mcs-daemon-foreground", "BEE]]", "FUSE]]"
        , "[[", "--mcs-daemon-foreground", "]]"
        , "--mcs-daemon-foreground"
        }
      };
    auto const embedded_arguments
      { arguments::View {pointers.data(), pointers.size() - 1UL}
      };
    auto parsed
      { daemon::detail::ParsedArguments
        { arguments::View {pointers.data(), pointers.size()}
        }
      };

    EXPECT_TRUE (parsed.foreground());

    auto const forwarded {static_cast<arguments::View> (parsed)};
    EXPECT_TRUE (std::ranges::equal (forwarded, embedded_arguments));

    auto embedded_only
      { daemon::detail::ParsedArguments {embedded_arguments}
      };

    EXPECT_FALSE (embedded_only.foreground());
    EXPECT_TRUE
      ( std::ranges::equal
        ( static_cast<arguments::View> (embedded_only)
        , embedded_arguments
        )
      );
  }

  TEST (MCSUtilDaemon, arguments_reject_empty_input)
  {
    auto const arguments {arguments::View {nullptr, 0UL}};
    auto invoked {false};

    testing::require_exception
      ( [&]
        {
          std::ignore = daemonize
            ( arguments
            , [&] (arguments::View, Daemon&) noexcept -> int
              {
                invoked = true;
                return EXIT_SUCCESS;
              }
            );
        }
      , testing::assert_type_and_what<std::logic_error>
          ("daemonize: empty args!?")
      );

    EXPECT_FALSE (invoked);
  }

  TEST (MCSUtilDaemon, arguments_propagate_foreground_exception)
  {
    auto const argv
      { std::array<char const*, 2>
        { "daemonize_test"
        , "--mcs-daemon-foreground"
        }
      };
    auto const arguments {arguments::View {argv.data(), argv.size()}};

    testing::require_exception
      ( [&]
        {
          std::ignore = daemonize
            ( arguments
            , [] (arguments::View, Daemon&) -> int
              {
                throw std::runtime_error {"expected foreground failure"};
              }
            );
        }
      , testing::assert_type_and_what<std::runtime_error>
          ("expected foreground failure")
      );
  }

  TEST (MCSUtilDaemon, arguments_require_foreground_ready_signal)
  {
    auto const argv
      { std::array<char const*, 2>
        { "daemonize_test"
        , "--mcs-daemon-foreground"
        }
      };
    auto const arguments {arguments::View {argv.data(), argv.size()}};

    testing::require_exception
      ( [&]
        {
          std::ignore = daemonize
            ( arguments
            , [] (arguments::View, Daemon&) noexcept -> int
              {
                return EXIT_SUCCESS;
              }
            );
        }
      , testing::assert_type_and_what<std::runtime_error>
          ( "daemonize: startup failed: daemon startup finished"
            " without calling ready() or fail()"
          )
      );
  }

  TEST (MCSUtilDaemon, arguments_forward_move_only_implementation)
  {
    auto argv
      { std::array<char const*, 2>
        { "daemonize_test"
        , "--mcs-daemon-foreground"
        }
      };
    auto const arguments {arguments::View {argv.data(), argv.size()}};
    auto const make_implementation
      { []
        {
          return [value {std::make_unique<int> (73)}]
              (arguments::View, Daemon& daemon) -> int
            {
              daemon.ready();

              return *value;
            };
        }
      };

    {
      auto implementation {std::invoke (make_implementation)};
      static_assert (!std::is_copy_constructible_v<decltype (implementation)>);
      static_assert (!noexcept (daemonize (arguments, implementation)));
      static_assert
        (noexcept (daemonize (2, argv.data(), test_help, implementation)));

      EXPECT_EQ (daemonize (arguments, implementation), 73);
      EXPECT_EQ (daemonize (arguments, std::move (implementation)), 73);
    }

    {
      auto implementation {std::invoke (make_implementation)};

      EXPECT_EQ
        ( daemonize
          (2, argv.data(), test_help, std::move (implementation))
        , 73
        );
    }
  }

  TEST (MCSUtilDaemon, command_line_help_precedes_daemonization)
  {
    auto argv
      { std::array<char const*, 3>
        {"daemonize_test", "invalid", "--help"}
      };
    auto help_called {false};
    auto main_called {false};

    auto const result
      { daemonize
        ( 3
        , argv.data()
        , [&] (arguments::View args) noexcept
          {
            help_called = args.help_requested();
          }
        , [&] (arguments::View, Daemon& daemon) -> int
          {
            main_called = true;
            daemon.ready();

            return EXIT_SUCCESS;
          }
        )
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
    EXPECT_TRUE (help_called);
    EXPECT_FALSE (main_called);
  }

  TEST (MCSUtilDaemon, startup_ready_returns_success_and_daemon_runs)
  {
    auto const temporary_directory
      { testing::UniqTemporaryDirectory {"daemonize_ok"}
      };

    auto const pid_path {temporary_directory.path() / "PID"};
    auto const stop_event_fd {testing::syscall::eventfd (0U, 0)};

    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      auto const argv
        { std::array<char const*, 2>
          { "daemonize_test"
          , "EXPECTED_ARGUMENT"
          }
        };

      auto const result
        { daemonize
          ( arguments::View {argv.data(), argv.size()}
          , [&] (arguments::View args, Daemon& daemon) -> int
            {
              if (args.size() != 2)
              {
                return EXIT_FAILURE;
              }

              if (std::string {args.at (1)} != "EXPECTED_ARGUMENT")
              {
                return EXIT_FAILURE;
              }

              FMT::write_file (pid_path, "{}", syscall::getpid());

              daemon.ready();

              if (read_event_counter (stop_event_fd) != 1U)
              {
                return EXIT_FAILURE;
              }

              return EXIT_SUCCESS;
            }
          )
        };

      testing::syscall::_exit (result);
    }

    ASSERT_NE (runner, -1);

    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
    EXPECT_TRUE (std::filesystem::exists (pid_path));

    auto const daemon_pid {read_pid (pid_path)};
    EXPECT_LT (0, daemon_pid);
    EXPECT_TRUE (process_exists (daemon_pid));

    write_event_counter (stop_event_fd, 1U);

    stop_daemon_and_expect_exit (daemon_pid);
  }

  TEST (MCSUtilDaemon, startup_ready_releases_standard_output_streams)
  {
    auto const temporary_directory
      { testing::UniqTemporaryDirectory {"daemonize_standard_streams"}
      };
    auto const pid_path {temporary_directory.path() / "PID"};
    auto const stdout_pipe {syscall::pipe()};
    auto const stderr_pipe {syscall::pipe()};
    auto const stop_event_fd {testing::syscall::eventfd (0U, 0)};
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      std::ignore = syscall::dup2
        ( stdout_pipe.write()
        , syscall::fileno (stdout)
        );
      std::ignore = syscall::dup2
        ( stderr_pipe.write()
        , syscall::fileno (stderr)
        );

      syscall::close (stdout_pipe.read());
      syscall::close (stdout_pipe.write());
      syscall::close (stderr_pipe.read());
      syscall::close (stderr_pipe.write());

      auto argv {std::array<char const*, 1> {"daemonize_test"}};
      auto const result
        { daemonize
          ( argv.size()
          , argv.data()
          , test_help
          , [&] (arguments::View, Daemon& daemon) -> int
            {
              FMT::write_file (pid_path, "{}", syscall::getpid());

              if ( std::fputs ("stdout before ready\n", stdout) == EOF
                || std::fputs ("stderr before ready\n", stderr) == EOF
                 )
              {
                return EXIT_FAILURE;
              }

              daemon.ready();

              return read_event_counter (stop_event_fd) == 1U
                ? EXIT_SUCCESS
                : EXIT_FAILURE
                ;
            }
          )
        };

      testing::syscall::_exit (result);
    }

    ASSERT_NE (runner, -1);

    syscall::close (stdout_pipe.write());
    syscall::close (stderr_pipe.write());

    auto stdout_eof
      { std::async (std::launch::async, read_until_eof, stdout_pipe.read())
      };
    auto stderr_eof
      { std::async (std::launch::async, read_until_eof, stderr_pipe.read())
      };

    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);

    auto const daemon_pid {read_pid (pid_path)};
    EXPECT_TRUE (process_exists (daemon_pid));

    auto const stdout_status
      { stdout_eof.wait_for (std::chrono::milliseconds {500})
      };
    auto const stderr_status
      { stderr_eof.wait_for (std::chrono::milliseconds {500})
      };

    write_event_counter (stop_event_fd, 1U);

    auto const stdout_output {stdout_eof.get()};
    auto const stderr_output {stderr_eof.get()};

    EXPECT_EQ (stdout_status, std::future_status::ready);
    EXPECT_EQ (stderr_status, std::future_status::ready);
    EXPECT_NE (stdout_output.find ("stdout before ready"), std::string::npos);
    EXPECT_NE (stderr_output.find ("stderr before ready"), std::string::npos);

    stop_daemon_and_expect_exit (daemon_pid);
  }

  TEST (MCSUtilDaemon, startup_ready_with_closed_standard_streams)
  {
    auto const temporary_directory
      { testing::UniqTemporaryDirectory {"daemonize_closed_streams"}
      };
    auto const pid_path {temporary_directory.path() / "PID"};
    auto const stop_event_fd {testing::syscall::eventfd (0U, 0)};
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      syscall::close (STDIN_FILENO);
      syscall::close (STDOUT_FILENO);
      syscall::close (STDERR_FILENO);

      auto argv {std::array<char const*, 1> {"daemonize_test"}};
      auto const result
        { daemonize
          ( argv.size()
          , argv.data()
          , test_help
          , [&] (arguments::View, Daemon& daemon) -> int
            {
              FMT::write_file (pid_path, "{}", syscall::getpid());

              daemon.ready();

              return read_event_counter (stop_event_fd) == 1U
                ? EXIT_SUCCESS
                : EXIT_FAILURE
                ;
            }
          )
        };

      testing::syscall::_exit (result);
    }

    ASSERT_NE (runner, -1);
    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);

    auto const daemon_pid {read_pid (pid_path)};
    EXPECT_TRUE (process_exists (daemon_pid));

    write_event_counter (stop_event_fd, 1U);

    stop_daemon_and_expect_exit (daemon_pid);
  }

  namespace
  {
    template<typename Fail>
      auto startup_fail_returns_exit_failure_and_reason
        ( std::vector<char const*> argv
        , Fail&& fail
        , std::string_view expected
        )
    {
      auto const runner {syscall::fork()};

      if (runner == 0)
      {
        auto const runner_process_id {syscall::getpid()};

        auto const stderr_pipe {syscall::pipe()};
        auto const stderr_fd {syscall::fileno (stderr)};
        auto const stderr_backup {syscall::dup (stderr_fd)};
        std::ignore = syscall::dup2 (stderr_pipe.write(), stderr_fd);

        syscall::close (stderr_pipe.write());

        auto const result
          { daemonize
            ( cast<int> (argv.size())
            , argv.data()
            , test_help
            , [&]
              ( arguments::View
              , Daemon& daemon
              ) noexcept (std::is_nothrow_invocable_v<Fail, Daemon&>) -> int
              {
                std::invoke (std::forward<Fail> (fail), daemon);

                return EXIT_SUCCESS;
              }
            )
          };

        // daemonize() returns in both the startup parent and the daemon child.
        // Only the startup parent should consume the stderr capture pipe.
        if (syscall::getpid() != runner_process_id)
        {
          testing::syscall::_exit (EXIT_SUCCESS);
        }

        syscall::fflush (stderr);

        std::ignore = syscall::dup2 (stderr_backup, stderr_fd);

        syscall::close (stderr_backup);

        auto message {read_until_eof (stderr_pipe.read())};

        auto const first {message.find (expected)};
        auto const second
          { first == std::string::npos
            ? std::string::npos
            : message.find (expected, first + expected.size())
          };

        if (  result == EXIT_FAILURE
           && first != std::string::npos
           && second == std::string::npos
           )
        {
          testing::syscall::_exit (EXIT_SUCCESS);
        }

        testing::syscall::_exit (EXIT_FAILURE);
      }

      ASSERT_NE (runner, -1);

      EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
    }
  }

  TEST (MCSUtilDaemon, startup_exception_returns_exit_failure)
  {
    startup_fail_returns_exit_failure_and_reason
      ( {"daemonize_test"}
      , [] (Daemon&)
        {
          throw std::runtime_error {"expected startup failure"};
        }
      , "expected startup failure"
      );
  }

  TEST (MCSUtilDaemon, arguments_propagate_startup_failure_to_parent)
  {
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      auto const argv {std::array<char const*, 1> {"daemonize_test"}};
      auto const arguments {arguments::View {argv.data(), argv.size()}};

      testing::require_exception
        ( [&]
          {
            std::ignore = daemonize
              ( arguments
              , [] (arguments::View, Daemon&) -> int
                {
                  throw std::runtime_error {"expected startup failure"};
                }
              );

            // The child returns; the startup parent must see an exception.
            testing::syscall::_exit (EXIT_FAILURE);
          }
        , testing::assert_type_and_what<std::runtime_error>
            ("daemonize: startup failed: expected startup failure")
        );

      testing::syscall::_exit
        ( ::testing::Test::HasFailure() ? EXIT_FAILURE : EXIT_SUCCESS
        );
    }

    ASSERT_NE (runner, -1);
    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
  }

  TEST (MCSUtilDaemon, startup_read_retries_when_interrupted)
  {
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      using SignalAction = struct ::sigaction;
      auto action {SignalAction{}};
      action.sa_handler = handle_signal;
      testing::syscall::sigemptyset (std::addressof (action.sa_mask));
      testing::syscall::sigaction (SIGUSR1, std::addressof (action), nullptr);

      auto const startup_parent {syscall::getpid()};
      auto const sender {syscall::fork()};

      if (sender == 0)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds {50});
        testing::syscall::kill (startup_parent, SIGUSR1);
        testing::syscall::_exit (EXIT_SUCCESS);
      }

      if (sender == -1)
      {
        testing::syscall::_exit (EXIT_FAILURE);
      }

      auto argv {std::array<char const*, 1> {"daemonize_test"}};
      auto const result
        { daemonize
          ( argv.size()
          , argv.data()
          , test_help
          , [] (arguments::View, Daemon& daemon) -> int
            {
              std::this_thread::sleep_for (std::chrono::milliseconds {500});
              daemon.ready();

              return EXIT_SUCCESS;
            }
          )
        };

      if (syscall::getpid() != startup_parent)
      {
        testing::syscall::_exit (result);
      }

      auto const sender_result {wait_for_exit (sender)};
      testing::syscall::_exit
        ( result == EXIT_SUCCESS && sender_result == EXIT_SUCCESS
          ? EXIT_SUCCESS
          : EXIT_FAILURE
        );
    }

    ASSERT_NE (runner, -1);

    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
  }

  TEST (MCSUtilDaemon, startup_write_retries_when_interrupted)
  {
    auto const blocked_event_fd {testing::syscall::eventfd (0U, 0)};
    auto const signal_event_fd {testing::syscall::eventfd (0U, 0)};
    auto const saturated_value
      { std::numeric_limits<std::uint64_t>::max() - 1U
      };

    EXPECT_EQ
      ( syscall::write
        ( blocked_event_fd
        , std::addressof (saturated_value)
        , sizeof (saturated_value)
        )
      , sizeof (saturated_value)
      );

    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      using SignalAction = struct ::sigaction;
      auto action {SignalAction{}};
      action.sa_handler = handle_signal;
      testing::syscall::sigemptyset (std::addressof (action.sa_mask));
      testing::syscall::sigaction (SIGUSR1, std::addressof (action), nullptr);

      signal_was_handled = 0;

      auto const writer {syscall::getpid()};
      auto const sender {syscall::fork()};

      if (sender == 0)
      {
        std::this_thread::sleep_for (std::chrono::milliseconds {50});
        testing::syscall::kill (writer, SIGUSR1);
        std::this_thread::sleep_for (std::chrono::milliseconds {50});
        write_event_counter (signal_event_fd, 1U);
        testing::syscall::_exit (EXIT_SUCCESS);
      }

      if (sender == -1)
      {
        testing::syscall::_exit (EXIT_FAILURE);
      }

      syscall::close (signal_event_fd);

      auto const write_succeeded
        { std::invoke
          ( [&]
            {
              try
              {
                auto const value {std::uint64_t {1}};
                return syscall::write_retrying_on_interrupt
                  ( blocked_event_fd
                  , std::addressof (value)
                  , sizeof (value)
                  ) == sizeof (value)
                  ;
              }
              catch (...)
              {
                return false;
              }
            }
          )
        };

      auto const sender_result {wait_for_exit (sender)};
      syscall::close (blocked_event_fd);

      testing::syscall::_exit
        (  write_succeeded
        && signal_was_handled != 0
        && sender_result == EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE
        );
    }

    ASSERT_NE (runner, -1);
    EXPECT_EQ (read_event_counter (signal_event_fd), 1U);

    auto drained_value {std::uint64_t {0}};
    EXPECT_EQ
      ( syscall::read
        ( blocked_event_fd
        , std::addressof (drained_value)
        , sizeof (drained_value)
        )
      , sizeof (drained_value)
      );
    EXPECT_EQ (drained_value, saturated_value);
    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
    EXPECT_EQ (read_event_counter (blocked_event_fd), 1U);
  }

  TEST (MCSUtilDaemon, startup_child_exit_reports_missing_status)
  {
    startup_fail_returns_exit_failure_and_reason
      ( {"daemonize_test"}
      , [] (Daemon&) noexcept -> void
        {
          testing::syscall::_exit (EXIT_FAILURE);
        }
      , "daemon exited before reporting startup status"
      );
  }

  TEST (MCSUtilDaemon, foreground_without_signal_returns_exit_failure)
  {
    startup_fail_returns_exit_failure_and_reason
      ( {"daemonize_test", "--mcs-daemon-foreground"}
      , [] (Daemon&) noexcept
        {
        }
      , "daemon startup finished without calling ready() or fail()"
      );
  }

  TEST (MCSUtilDaemon, duplicate_ready_throws_startup_signal_already_sent)
  {
    auto argv
      { std::array<char const*, 2>
        { "daemonize_test"
        , "--mcs-daemon-foreground"
        }
      };

    auto const result
      { daemonize
        ( argv.size()
        , argv.data()
        , test_help
        , [] (arguments::View, Daemon& daemon) -> int
          {
            daemon.ready();

            try
            {
              daemon.ready();
            }
            catch (Daemon::Error::MultipleSignals const&)
            {
              return EXIT_SUCCESS;
            }

            return EXIT_FAILURE;
          }
        )
      };

    EXPECT_EQ (result, EXIT_SUCCESS);
  }

  TEST (MCSUtilDaemon, throw_after_ready_returns_startup_success)
  {
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      auto argv {std::array<char const*, 1> {"daemonize_test"}};

      auto const result
        { daemonize
          ( argv.size()
          , argv.data()
          , test_help
          , [] (arguments::View, Daemon& daemon) -> int
            {
              daemon.ready();

              throw std::runtime_error {"expected throw after ready"};

              return EXIT_FAILURE;
            }
          )
        };

      testing::syscall::_exit (result);
    }

    ASSERT_NE (runner, -1);

    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
  }

  TEST (MCSUtilDaemon, startup_without_signal_returns_default_failure)
  {
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      auto argv {std::array<char const*, 1> {"daemonize_test"}};

      auto const result
        { daemonize
          ( argv.size()
          , argv.data()
          , test_help
          , [] (arguments::View, Daemon&) noexcept -> int
            {
              return EXIT_SUCCESS;
            }
          )
        };

      if (result == EXIT_FAILURE)
      {
        testing::syscall::_exit (EXIT_SUCCESS);
      }

      testing::syscall::_exit (EXIT_FAILURE);
    }

    ASSERT_NE (runner, -1);

    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
  }

  TEST (MCSUtilDaemon, foreground_option_keeps_runner_alive_until_main_returns)
  {
    auto const started_event_fd {testing::syscall::eventfd (0U, 0)};
    auto const stop_event_fd {testing::syscall::eventfd (0U, 0)};
    auto const runner {syscall::fork()};

    if (runner == 0)
    {
      auto argv
        { std::array<char const*, 2>
          { "daemonize_test"
          , "--mcs-daemon-foreground"
          }
        };

      auto const result
        { daemonize
          ( argv.size()
          , argv.data()
          , test_help
          , [&] (arguments::View, Daemon& daemon) -> int
            {
              daemon.ready();

              write_event_counter (started_event_fd, 1U);

              if (read_event_counter (stop_event_fd) != 1U)
              {
                return EXIT_FAILURE;
              }

              return EXIT_SUCCESS;
            }
          )
        };

      testing::syscall::_exit (result);
    }

    ASSERT_NE (runner, -1);
    EXPECT_EQ (read_event_counter (started_event_fd), 1U);

    auto status {int {0}};
    EXPECT_EQ
      ( testing::syscall::waitpid (runner, std::addressof (status), WNOHANG)
      , 0
      );
    EXPECT_TRUE (process_exists (runner));

    write_event_counter (stop_event_fd, 1U);

    EXPECT_EQ (wait_for_exit (runner), EXIT_SUCCESS);
  }
}
