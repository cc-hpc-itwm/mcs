// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/util/read/read.hpp>
#include <utility>

namespace mcs::testing
{
  template<read_fmt_identity_testable T>
    constexpr auto read_of_fmt_is_identity (T const& x)
  {
    auto const formatted {fmt::format ("{}", x)};

    ASSERT_EQ (x, util::read::read<T> (formatted));
  }

  template< read_fmt_identity_testable_with_custom_checker T
          , typename Checker
          >
    requires (is_checker<T, Checker>)
    constexpr auto read_of_fmt_is_identity (T const& x, Checker&& check)
  {
    auto const formatted {fmt::format ("{}", x)};

    ASSERT_TRUE
      ( std::forward<Checker> (check) (util::read::read<T> (formatted))
      );
  }
}
