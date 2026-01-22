// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>

namespace mcs::fuse
{
  // A main function parses a command line and runs the fuse loop. If
  // the command line parsing fails or if help is request, then
  // print_usage is invoked.
  //
  template<is_content Content, typename PrintUsage>
    auto main ( PrintUsage
              , int argc
              , char** argv
              , typename Content::State
              ) -> int
    ;
}

#include "detail/main.ipp"
