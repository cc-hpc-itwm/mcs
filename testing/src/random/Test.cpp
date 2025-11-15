// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdio>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/seed.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>

namespace mcs::testing::random
{
  Test::~Test() noexcept
  {
    if (HasFailure())
    {
      util::FMT::print_noexcept
        ( stdout
        , "Random number generator was initialized with: {}\n"
        , seed()
        );
    }
  }
}
