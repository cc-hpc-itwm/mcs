// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdio>
#include <fmt/format.h>
#include <mcs/nonstd/scope/exit.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/daemon.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/dup.hpp>
#include <mcs/util/syscall/dup2.hpp>
#include <mcs/util/syscall/fflush.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/write_retrying_on_interrupt.hpp>
#include <optional>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <vector>

namespace mcs::util::daemon::detail
{
  ParsedArguments::ParsedArguments (arguments::View args)
    : _arguments {args}
    , _foreground {args.contains_any_of ({option::foreground})}
  {
    if (args.size() == 0UL)
    {
      throw std::logic_error {"daemonize: empty args!?"};
    }

    _arguments -= option::foreground;
  }

  ParsedArguments::operator arguments::View() noexcept
  {
    return _arguments.view();
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::util::daemon::detail::status::Fail>::output
    ( OArchive& archive
    , Type const& fail
    ) -> OArchive&
  {
    return save (archive, fail.reason);
  }

  auto Implementation<mcs::util::daemon::detail::status::Fail>::input
    ( IArchive& archive
    ) -> Type
  {
    auto reason {load<decltype (Type::reason)> (archive)};

    return Type {std::move (reason)};
  }
}

namespace mcs::util::daemon::detail
{
  auto write_status
    ( std::optional<int>& fd
    , Status status
    ) -> void
  {
    if (fd.has_value())
    {
      mcs::serialization::OArchive {status}.for_each_data_buffer
        ( [&] (auto buffer)
          {
            while (!buffer.empty())
            {
              buffer = buffer.subspan
                ( cast<std::size_t>
                  ( syscall::write_retrying_on_interrupt
                    (*fd, buffer.data(), buffer.size())
                  )
                );
            }
          }
        );

      syscall::close (*fd);

      fd.reset();
    }
  }
}

namespace mcs::util
{
  Daemon::Error::MultipleSignals::MultipleSignals()
    : mcs::Error {"Daemon::ready()/fail() must be called at most once"}
  {}

  Daemon::Error::MultipleSignals::~MultipleSignals() = default;

  namespace
  {
    auto close_noexcept (int fd) noexcept -> void
    {
      try
      {
        syscall::close (fd);
      }
      catch (...) // NOLINT (bugprone-empty-catch)
      {
      }
    }

    auto protect_status_descriptor (int& fd) -> void
    {
      if (fd > STDERR_FILENO)
      {
        return;
      }

      auto low_descriptors {std::vector<int>{}};
      low_descriptors.reserve (3);

      auto const close_low_descriptors
        { nonstd::make_scope_exit
          ( [&]() noexcept
            {
              std::ranges::for_each (low_descriptors, close_noexcept);
            }
          )
        };

      auto duplicate {syscall::dup (fd)};

      while (! (duplicate > STDERR_FILENO))
      {
        low_descriptors.emplace_back (duplicate);
        duplicate = syscall::dup (fd);
      }

      auto const original {fd};
      fd = duplicate;

      syscall::close (original);
    }

    auto detach_standard_streams() -> void
    {
      syscall::fflush (stdout);
      syscall::fflush (stderr);

      auto null_stream {util::fopen ("/dev/null", "r+")};
      auto const null_fd {syscall::fileno (null_stream.get())};

      std::ignore = syscall::dup2 (null_fd, STDIN_FILENO);
      std::ignore = syscall::dup2 (null_fd, STDOUT_FILENO);
      std::ignore = syscall::dup2 (null_fd, STDERR_FILENO);

      if (! (null_fd > STDERR_FILENO))
      {
        auto const source_fd
          { null_fd == STDIN_FILENO ? STDOUT_FILENO : STDIN_FILENO
          };

        null_stream.reset();
        std::ignore = syscall::dup2 (source_fd, null_fd);
      }
    }
  }

  auto Daemon::fail (std::string reason) -> void
  {
    if (_write_end.has_value())
    {
      daemon::detail::write_status
        ( _write_end
        , daemon::detail::status::Fail {std::move (reason)}
        );
    }
    else
    {
      _foreground_failure_reason.emplace (std::move (reason));
    }
  }

  auto Daemon::ready() -> void
  {
    if (_write_end.has_value())
    {
      protect_status_descriptor (*_write_end);
      detach_standard_streams();
    }

    if (!_signal_pending)
    {
      throw Error::MultipleSignals{};
    }

    daemon::detail::write_status (_write_end, daemon::detail::status::Ready{});
  }
}
