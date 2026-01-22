// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/session/option/Clone_fd.hpp>
#include <mcs/fuse/session/option/Foreground.hpp>
#include <mcs/fuse/session/option/MaxThreads.hpp>
#include <mcs/fuse/session/option/Mountpoint.hpp>
#include <mcs/fuse/session/option/Singlethread.hpp>
#include <mcs/util/not_null.hpp>
#include <variant>

namespace mcs::fuse::session
{
  struct Options
  {
    template<typename Option>
      using Flag = std::variant<typename Option::Yes, typename Option::No>
      ;

    [[nodiscard]] Options
      ( option::Mountpoint
      , Flag<option::Singlethread>
      , Flag<option::Foreground>
      , Flag<option::Clone_fd>
      , option::MaxThreads
      )
      ;
    [[nodiscard]] Options (util::not_null<::fuse_cmdline_opts>);

    [[nodiscard]] auto mountpoint() -> char const*;
    [[nodiscard]] auto singlethread() const -> int;
    [[nodiscard]] auto foreground() const -> int;
    [[nodiscard]] auto clone_fd() const -> unsigned int;
    [[nodiscard]] auto max_threads() const -> unsigned;

  private:
    option::Mountpoint _mountpoint;
    Flag<option::Singlethread> _singlethread;
    Flag<option::Foreground> _foreground;
    Flag<option::Clone_fd> _clone_fd;
    option::MaxThreads _max_threads;
  };
}
