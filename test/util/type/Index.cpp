// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/util/type/Index.hpp>
#include <type_traits>

namespace mcs::util::type
{
  TEST (UtilTypeIndex, computes_minimal_index_of_type)
  {
    static_assert (Index<int, int, char, int>::value == 0);
    static_assert (Index<char, int, char, int>::value == 1);
  }
}
