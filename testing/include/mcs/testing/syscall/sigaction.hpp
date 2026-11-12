// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <signal.h>

namespace mcs::testing::syscall
{
  auto sigaction
    ( int
    , struct ::sigaction const*
    , struct ::sigaction*
    ) -> void;
}
