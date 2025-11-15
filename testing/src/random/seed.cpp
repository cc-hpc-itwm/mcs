// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/seed.hpp>

namespace mcs::testing::random
{
  auto seed() -> decltype ( std::chrono::high_resolution_clock::now()
                          . time_since_epoch().count()
                          )
  {
    static auto const seed ( std::chrono::high_resolution_clock::now()
                           . time_since_epoch().count()
                           );
    return seed;
  }
}
