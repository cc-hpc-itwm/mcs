// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/util/type/At.hpp>
#include <type_traits>

namespace mcs::util::type
{
  TEST (UtilTypeAt, selects_nth_elements)
  {
    static_assert (std::is_same_v<At<0, int, char>::type, int>);
    static_assert (std::is_same_v<At<1, int, char>::type, char>);
    static_assert (std::is_same_v<At<2, int, char, int>::type, int>);
    static_assert (!std::is_same_v<At<2, int, char, int>::type, char>);
  }
}
