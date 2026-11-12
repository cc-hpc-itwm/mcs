// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <iterator>
#include <mcs/nonstd/scope/exit.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/variant.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/arguments/Owned.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/fork.hpp>
#include <mcs/util/syscall/pipe.hpp>
#include <mcs/util/syscall/read_retrying_on_interrupt.hpp>
#include <mcs/util/syscall/setsid.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace mcs::util::daemon::detail
{
  struct ParsedArguments
  {
    [[nodiscard]] explicit ParsedArguments (arguments::View);
    [[nodiscard]] operator arguments::View() noexcept;
    [[nodiscard]] constexpr auto foreground() const noexcept -> bool;

  private:
    arguments::Owned _arguments;
    bool _foreground;
  };
}

namespace mcs::util::daemon::detail
{
  namespace status
  {
    struct Ready{};
    struct Fail
    {
      std::string reason{};
    };
  }

  using Status = std::variant<status::Ready, status::Fail>;

  auto write_status (std::optional<int>&, Status) -> void;
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::util::daemon::detail::status::Fail>
  {
    using Type = mcs::util::daemon::detail::status::Fail;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util
{
  constexpr Daemon::Daemon (int write_end)
    : _write_end {write_end}
  {}

  template<is_daemonized_main_implementation Main>
    auto Daemon::run (arguments::View args, Main&& main_implementation) -> int
    {
      auto const result
        { std::invoke
          ( std::forward<Main> (main_implementation)
          , args
          , *this
          )
        };

      if (_signal_pending)
      {
        fail ("daemon startup finished without calling ready() or fail()");
      }

      if (_foreground_failure_reason.has_value())
      {
        throw std::runtime_error
          { fmt::format
            ( "daemonize: startup failed: {}"
            , *_foreground_failure_reason
            )
          };
      }

      return result;
    }

  template<is_daemonized_main_implementation Main>
    auto daemonize
      ( arguments::View args
      , Main&& main_implementation
      ) -> int
    {
      if ( auto parsed_arguments {daemon::detail::ParsedArguments {args}}
         ; parsed_arguments.foreground()
         )
      {
        auto daemon {Daemon{}};

        return daemon.run
          ( parsed_arguments
          , std::forward<Main> (main_implementation)
          );
      }
      else
      {
        auto const parent
          { [] (int read_fd)
            {
              auto const read_status
                { [&]
                  {
                    auto bytes {std::vector<std::byte>{}};

                    {
                      auto const close_read_fd
                        { nonstd::make_scope_exit
                          ( [read_fd]() noexcept
                            {
                              try
                              {
                                syscall::close (read_fd);
                              }
                              catch (...) // NOLINT (bugprone-empty-catch)
                              {
                              }
                            }
                          )
                        };

                      auto buffer {std::array<std::byte, 1 << 10>{}};

                      auto const read_chunk
                        { [&]
                          {
                            return std::span<std::byte>
                              { buffer.data()
                              , buffer.data()
                              + syscall::read_retrying_on_interrupt
                                ( read_fd
                                , buffer.data()
                                , buffer.size()
                                )
                              };
                          }
                        };

                      for ( auto chunk {read_chunk()}
                          ; !chunk.empty()
                          ; chunk = read_chunk()
                          )
                      {
                        std::ranges::copy
                          ( chunk
                          , std::back_inserter (bytes)
                          );
                      }
                    }

                    if (bytes.empty())
                    {
                      throw std::runtime_error
                        { "daemonize: daemon exited before reporting"
                          " startup status"
                        };
                    }

                    try
                    {
                      using Status = daemon::detail::Status;
                      return serialization::load_from<Status> (bytes);
                    }
                    catch (...)
                    {
                      std::throw_with_nested
                        ( std::runtime_error
                          { "daemonize: deserialize status bytes"
                          }
                        );
                    }
                  }
                };

              return std::visit
                ( overloaded
                  { [&] (daemon::detail::status::Ready) noexcept -> int
                    {
                      return EXIT_SUCCESS;
                    }
                  , [&] (daemon::detail::status::Fail fail) -> int
                    {
                      throw std::runtime_error
                        { fmt::format ( "daemonize: startup failed: {}"
                                      , fail.reason
                                      )
                        };
                    }
                  }
                , std::invoke (read_status)
                );
            }
          };

        auto const child
          { [&] (int write_fd)
            {
              auto daemon {Daemon {write_fd}};

              try
              {
                std::ignore = syscall::setsid();

                return daemon.run
                  ( parsed_arguments
                  , std::forward<Main> (main_implementation)
                  );
              }
              catch (...)
              {
                daemon.fail
                  ( fmt::format ("{}", std::current_exception())
                  );

                return EXIT_FAILURE;
              }
            }
        };

        auto const pipe {syscall::pipe()};

        if (syscall::fork() > 0)
        {
          syscall::close (pipe.write());

          return std::invoke (parent, pipe.read());
        }
        else
        {
          syscall::close (pipe.read());

          return std::invoke (child, pipe.write());
        }
      }
    }

  template< is_help_implementation Help
          , is_daemonized_main_implementation Main
          >
    auto daemonize
      ( int argc
      , char const** argv
      , Help&& help
      , Main&& main_implementation
      ) noexcept -> int
    {
      return main
        ( argc
        , argv
        , std::forward<Help> (help)
        , [&] (arguments::View args) -> int
          {
            return daemonize
              ( args
              , std::forward<Main> (main_implementation)
              );
          }
        );
    }
}

namespace mcs::util::daemon::detail
{
  constexpr auto ParsedArguments::foreground() const noexcept -> bool
  {
    return _foreground;
  }
}
