// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/value/UTIL/string.hpp>

namespace mcs::testing::random
{
  auto value<mcs::util::string>::operator()() -> mcs::util::string
  {
    return mcs::util::string {_random_string()};
  }
}
