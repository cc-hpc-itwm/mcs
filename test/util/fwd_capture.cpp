// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/fwd_capture.hpp>

namespace
{
  struct S
  {
    S (int x) : _x {x} {}
    int _x;
  };

  auto g (auto const& s)
  {
    return [s = mcs::util::fwd_capture (std::forward<decltype (s)> (s))]
      ( int k
      ) noexcept
    {
      return s.get()._x + k;
    };
  }

  auto f (auto&& s)
  {
    return [s = mcs::util::fwd_capture (std::forward<decltype (s)> (s))]
      ( int k
      ) mutable noexcept
    {
      return s.get()._x += k;
    };
  }
}

namespace mcs::util
{
  struct UtilFWDCaptureR : public testing::random::Test
  {
    testing::random::value<int> _random_int {-1000, 1000};
  };

  TEST_F (UtilFWDCaptureR, mutable_references_are_forwarded_to_mutable_lambda)
  {
    auto const x {std::invoke (_random_int)};
    auto const y {std::invoke (_random_int)};

    auto s {S {x}};

    ASSERT_EQ (std::invoke (f (s), y), x + y);
    ASSERT_EQ (s._x, x + y);
  }

  TEST_F (UtilFWDCaptureR, rvalues_are_forwarded_to_mutable_lambda)
  {
    auto const x {std::invoke (_random_int)};
    auto const y {std::invoke (_random_int)};

    ASSERT_EQ (std::invoke (f (S {x}), y), x + y);
  }

  TEST_F (UtilFWDCaptureR, mutable_references_are_forwarded_to_const_lambda)
  {
    auto const x {std::invoke (_random_int)};
    auto const y {std::invoke (_random_int)};

    auto s {S {x}};

    ASSERT_EQ (std::invoke (g (s), y), x + y);
    ASSERT_EQ (s._x, x);
  }

  TEST_F (UtilFWDCaptureR, rvalues_are_forwarded_to_const_lambda)
  {
    auto const x {std::invoke (_random_int)};
    auto const y {std::invoke (_random_int)};

    ASSERT_EQ (std::invoke (g (S {x}), y), x + y);
  }

  TEST_F (UtilFWDCaptureR, const_references_are_forwarded_to_const_lambda)
  {
    auto const x {std::invoke (_random_int)};
    auto const y {std::invoke (_random_int)};

    auto const s {S {x}};

    ASSERT_EQ (std::invoke (g (s), y), x + y);
    ASSERT_EQ (s._x, x);
  }
}
