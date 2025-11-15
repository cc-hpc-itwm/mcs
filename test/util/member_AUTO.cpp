// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <gtest/gtest.h>
#include <mcs/util/member_AUTO.hpp>
#include <type_traits>

namespace
{
  template<typename> struct F;

  template<> struct F<int>
  {
    [[nodiscard]] constexpr auto operator() (int i) const noexcept
    {
      return 2 * i;
    }
  };
  template<> struct F<long>
  {
    [[nodiscard]] constexpr auto operator() (long) const noexcept
    {
      return "beep";
    }
  };

  template<typename T>
    struct S
  {
    T _x;
    MCS_UTIL_MEMBER_AUTO (_y, std::invoke (F<decltype (_x)>{}, _x));
  };

  TEST (UtilMemberAUTO, documentation_example_works)
  {
    static_assert (std::is_same_v<decltype (S<int>::_y), int>);
    static_assert (std::is_same_v<decltype (S<long>::_y), char const*>);
  }
}
