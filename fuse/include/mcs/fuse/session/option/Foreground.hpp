// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::fuse::session::option
{
  struct Foreground
  {
    struct Yes
    {
      static constexpr auto value {0};
    };
    struct No
    {
      static constexpr auto value {1};
    };
  };
}
