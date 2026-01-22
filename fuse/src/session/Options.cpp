// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/fuse/session/Options.hpp>
#include <mcs/util/overloaded.hpp>

namespace mcs::fuse::session
{
  Options::Options ( option::Mountpoint mountpoint
                   , Flag<option::Singlethread> singlethread
                   , Flag<option::Foreground> foreground
                   , Flag<option::Clone_fd> clone_fd
                   , option::MaxThreads max_threads
                   )
    : _mountpoint {mountpoint}
    , _singlethread {singlethread}
    , _foreground {foreground}
    , _clone_fd {clone_fd}
    , _max_threads {max_threads}
  {}
}

namespace mcs::fuse::session
{
  namespace
  {
    template<typename Flag>
      [[nodiscard]] auto flag
        ( util::not_null<::fuse_cmdline_opts> options
        ) -> typename Options::Flag<Flag>
    {
      if (options->clone_fd)
      {
        return typename Flag::Yes{};
      }

      return typename Flag::No{};
    }
  }

  Options::Options (util::not_null<::fuse_cmdline_opts> options)
    : Options
      { option::Mountpoint {options->mountpoint}
      , flag<option::Singlethread> (options)
      , flag<option::Foreground> (options)
      , flag<option::Clone_fd> (options)
      , option::MaxThreads {options->max_idle_threads}
      }
  {}
}

namespace mcs::fuse::session
{
  namespace
  {
    template<typename Flag>
      [[nodiscard]] constexpr auto value (Flag const& flag)
    {
      return std::visit
        ( util::overloaded
          { [] (auto x) noexcept
            {
              return decltype (x)::value;
            }
          }
        , flag
        );
    }
  }

  auto Options::mountpoint() -> char const*
  {
    return _mountpoint.value;
  }
  auto Options::singlethread() const -> int
  {
    return value (_singlethread);
  }
  auto Options::foreground() const -> int
  {
    return value (_foreground);
  }
  auto Options::clone_fd() const -> unsigned int
  {
    return value (_clone_fd);
  }
  auto Options::max_threads() const -> unsigned
  {
    return _max_threads.value;
  }
}
