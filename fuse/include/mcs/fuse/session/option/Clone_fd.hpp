// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::fuse::session::option
{
  struct Clone_fd
  {
    struct Yes
    {
      static constexpr auto value {1u};
    };
    struct No
    {
      static constexpr auto value {0u};
    };
  };
}
