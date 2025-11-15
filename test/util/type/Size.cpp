// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/util/type/Size.hpp>
#include <type_traits>

namespace mcs::util::type
{
  TEST (UtilTypeSize, computes_the_number_of_types_in_a_list)
  {
    static_assert (Size<>::value == 0);
    static_assert (Size<int>::value == 1);
    static_assert (Size<char>::value == 1);
    static_assert (Size<char, int>::value == 2);
    static_assert (Size<int, char, int>::value == 3);
    static_assert (Size<char, int, char, int>::value == 4);
  }
}
