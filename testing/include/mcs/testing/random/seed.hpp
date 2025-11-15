// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <chrono>

namespace mcs::testing::random
{
  // sets this seed at startup
  //
  auto seed() -> decltype ( std::chrono::high_resolution_clock::now()
                          . time_since_epoch().count()
                          )
    ;
}
