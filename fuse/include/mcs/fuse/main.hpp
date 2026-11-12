// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>
#include <mcs/util/arguments/View.hpp>

namespace mcs::fuse
{
  // A main function parses a command line and runs the fuse loop. If
  // the command line parsing fails or if help is requested, then
  // print_usage is invoked. Caller's arguments are not modified.
  //
  template<is_content Content, typename PrintUsage>
    [[nodiscard]] auto main
      ( PrintUsage
      , util::arguments::View
      , typename Content::State
      ) -> int;

  // Invoke ready after mounting and starting the event loop.
  //
  template<is_content Content, typename PrintUsage, typename Ready>
    [[nodiscard]] auto main
      ( PrintUsage
      , util::arguments::View
      , typename Content::State
      , Ready
      ) -> int;
}

#include "detail/main.ipp"
