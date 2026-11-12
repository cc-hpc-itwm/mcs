// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/command_line/option/Flag.hpp>

namespace mcs::util::daemon::option
{
  inline constexpr auto foreground {"--mcs-daemon-foreground"};
  inline constexpr auto foreground_name
    { command_line::option::Flag {foreground + 2}
    };
}
