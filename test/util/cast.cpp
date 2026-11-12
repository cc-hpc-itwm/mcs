// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <concepts>
#include <cstdint>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <limits>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <utility>

namespace mcs::util
{
  namespace
  {
    static_assert (std::derived_from<CastOutOfRange, mcs::Error>);

    struct UtilCastF : public testing::random::Test{};
  }

  TEST (UtilCast, integer_casts_remain_constant_expressions)
  {
    constexpr auto maximum {std::numeric_limits<int>::max()};
    constexpr auto unsigned_maximum {static_cast<unsigned> (maximum)};
    constexpr auto unsigned_to_unsigned
      { util::cast<unsigned long> (unsigned_maximum)
      };
    constexpr auto signed_to_unsigned {util::cast<unsigned> (maximum)};
    constexpr auto unsigned_to_signed {util::cast<int> (unsigned_maximum)};
    constexpr auto signed_to_signed {util::cast<long> (maximum)};

    static_assert (std::cmp_equal (unsigned_to_unsigned, maximum));
    static_assert (std::cmp_equal (signed_to_unsigned, maximum));
    static_assert (std::cmp_equal (unsigned_to_signed, maximum));
    static_assert (std::cmp_equal (signed_to_signed, maximum));
  }

  TEST_F (UtilCastF, casting_a_negative_signed_to_an_unsigned_throws)
  {
    using RandomInt = testing::random::value<int>;
    auto const i {RandomInt {RandomInt::Max {-1}}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = util::cast<unsigned> (i);
        }
      , testing::assert_type_and_what<CastOutOfRange>
          ( fmt::format ("cast signed to unsigned: {} < 0", i)
          )
      );
  }

  TEST (UtilCast, casting_a_too_large_signed_to_an_unsigned_throws)
  {
    using Small = std::uint32_t;
    using Large = std::int64_t;
    static_assert (std::unsigned_integral<Small>);
    static_assert (std::signed_integral<Large>);
    static_assert (sizeof (Small) < sizeof (Large));

    auto const value {std::numeric_limits<Large>::max()};

    testing::require_exception
      ( [&]
        {
          std::ignore = util::cast<Small> (value);
        }
      , testing::assert_type_and_what<CastOutOfRange>
          ( fmt::format
            ( "cast signed to unsigned: {} > {}"
            , value
            , std::numeric_limits<Small>::max()
            )
          )
      );
  }

  TEST_F (UtilCastF, casting_a_nonnegative_signed_to_an_unsigned_works)
  {
    using RandomInt = testing::random::value<int>;
    auto const i {RandomInt {RandomInt::Min {0}}()};
    auto const u {util::cast<unsigned> (i)};

    ASSERT_TRUE (std::cmp_equal (i, u));
  }

  TEST (UtilCast, casting_a_signed_zero_to_an_unsigned_works)
  {
    auto const i {0};
    auto const u {util::cast<unsigned> (i)};

    ASSERT_TRUE (std::cmp_equal (i, u));
  }

  TEST (UtilCast, casting_the_max_nonnegative_signed_to_an_unsigned_works)
  {
    auto const i {std::numeric_limits<int>::max()};
    auto const u {util::cast<unsigned> (i)};

    ASSERT_TRUE (std::cmp_equal (i, u));
  }

  TEST_F (UtilCastF, casting_an_unsigned_to_itself_works)
  {
    auto const value {std::invoke (testing::random::value<unsigned>{})};

    EXPECT_EQ (util::cast<unsigned> (value), value);
  }

  TEST_F (UtilCastF, casting_an_unsigned_to_a_larger_unsigned_works)
  {
    using Small = std::uint8_t;
    using Large = std::uint64_t;
    static_assert (std::unsigned_integral<Small>);
    static_assert (std::unsigned_integral<Large>);
    static_assert (sizeof (Small) < sizeof (Large));

    auto const value {std::invoke (testing::random::value<Small>{})};

    EXPECT_TRUE (std::cmp_equal (util::cast<Large> (value), value));
  }

  TEST_F (UtilCastF, casting_a_fitting_unsigned_to_a_smaller_unsigned_works)
  {
    using Small = std::uint8_t;
    using Large = std::uint64_t;
    static_assert (std::unsigned_integral<Small>);
    static_assert (std::unsigned_integral<Large>);
    static_assert (sizeof (Small) < sizeof (Large));

    using RandomLarge = testing::random::value<Large>;
    auto const value
      { std::invoke
        ( RandomLarge {RandomLarge::Max {std::numeric_limits<Small>::max()}}
        )
      };

    EXPECT_TRUE (std::cmp_equal (util::cast<Small> (value), value));
  }

  TEST_F (UtilCastF, casting_a_non_fitting_unsigned_to_unsigned_throws)
  {
    using Small = std::uint8_t;
    using Large = std::uint64_t;
    static_assert (std::unsigned_integral<Small>);
    static_assert (std::unsigned_integral<Large>);
    static_assert (sizeof (Small) < sizeof (Large));

    using RandomLarge = testing::random::value<Large>;
    auto const value
      { std::invoke
        ( RandomLarge
          { RandomLarge::Min
            { util::cast<Large> (std::numeric_limits<Small>::max()) + Large {1}
            }
          }
        )
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = util::cast<Small> (value);
        }
      , testing::assert_type_and_what<CastOutOfRange>
          ( fmt::format
            ( "cast unsigned to unsigned: {} > {}"
            , value
            , std::numeric_limits<Small>::max()
            )
          )
      );
  }

  TEST (UtilCast, casting_a_signed_to_itself_works)
  {
    using RandomInt = testing::random::value<int>;
    auto const i {std::invoke (RandomInt {RandomInt::Min {0}})};

    ASSERT_TRUE (std::cmp_equal (i, util::cast<int> (i)));
  }

  TEST (UtilCast, casting_a_signed_to_an_at_least_the_same_size_signed_works)
  {
    using RandomInt = testing::random::value<int>;
    auto const i {std::invoke (RandomInt {RandomInt::Min {0}})};

    ASSERT_TRUE (sizeof (long) >= sizeof (int));
    ASSERT_TRUE (std::cmp_equal (i, util::cast<long> (i)));
  }

  TEST (UtilCast, casting_a_fitting_signed_to_a_smaller_signed_works)
  {
    using RandomLong = testing::random::value<long>;
    auto const l
      { std::invoke
        ( RandomLong { RandomLong::Min {0}
                     , RandomLong::Max {std::numeric_limits<int>::max()}
                     }
        )
      };

    ASSERT_TRUE (std::cmp_equal (l, util::cast<int> (l)));
  }

  TEST (UtilCast, casting_a_non_fitting_signed_to_a_smaller_signed_throws)
  {
    using RandomLong = testing::random::value<long>;
    auto const l
      { std::invoke
        ( RandomLong
          { RandomLong::Min
            { util::cast<long> (std::numeric_limits<int>::max()) + 1L
            }
          }
        )
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = util::cast<int> (l);
        }
      , testing::assert_type_and_what<CastOutOfRange>
         ( fmt::format ( "cast signed to signed: {} > {}"
                       , l
                       , std::numeric_limits<int>::max()
                       )
         )
      );
  }

  TEST (UtilCast, casting_a_too_small_signed_to_a_smaller_signed_throws)
  {
    auto const value
      { util::cast<long> (std::numeric_limits<int>::min()) - 1L
      };

    testing::require_exception
      ( [&]
        {
          std::ignore = util::cast<int> (value);
        }
      , testing::assert_type_and_what<CastOutOfRange>
          ( fmt::format
            ( "cast signed to signed: {} < {}"
            , value
            , std::numeric_limits<int>::min()
            )
          )
      );
  }

  TEST_F
    ( UtilCastF
    , casting_an_unsigned_too_large_than_a_signed_can_hold_throws
    )
  {
    using RandomUnsigned = testing::random::value<unsigned>;
    auto const max_int {util::cast<unsigned> (std::numeric_limits<int>::max())};
    auto const u {RandomUnsigned {RandomUnsigned::Min {max_int + 1u}}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = util::cast<int> (u);
        }
      , testing::assert_type_and_what<CastOutOfRange>
         ( fmt::format ( "cast unsigned to signed: {} > {}"
                       , u
                       , std::numeric_limits<int>::max()
                       )
         )
      );
  }

  TEST_F
    ( UtilCastF
    , casting_an_unsigned_small_enough_that_a_signed_can_hold_it_works
    )
  {
    using RandomUnsigned = testing::random::value<unsigned>;
    auto const max_int {util::cast<unsigned> (std::numeric_limits<int>::max())};
    auto const u {RandomUnsigned {RandomUnsigned::Max {max_int}}()};
    auto const i {util::cast<int> (u)};

    ASSERT_TRUE (std::cmp_equal (i, u));
  }

  TEST (UtilCast, casting_an_unsigned_zero_to_a_signed_works)
  {
    auto const u {0u};
    auto const i {util::cast<int> (u)};

    ASSERT_TRUE (std::cmp_equal (i, u));
  }

  TEST (UtilCast, casting_an_unsigned_max_signed_to_a_signed_works)
  {
    auto const u {util::cast<unsigned> (std::numeric_limits<int>::max())};
    auto const i {util::cast<int> (u)};

    ASSERT_TRUE (std::cmp_equal (i, u));
  }

  TEST_F
    ( UtilCastF
    , if_alignment_of_target_type_is_not_more_restrictive_than_alignment_of_source_type_then_pointer_cast_does_not_change_the_pointer_value
    )
  {
    static_assert (alignof (float) <= alignof (int));

    auto const i {testing::random::value<int>{}()};
    auto const f_ptr {util::cast<float const*> (std::addressof (i))};

    ASSERT_EQ
      ( static_cast<void const*> (std::addressof (i))
      , static_cast<void const*> (f_ptr)
      );
  }

  TEST_F
    ( UtilCastF
    , if_alignment_of_target_type_is_not_more_restrictive_than_alignment_of_source_type_then_pointer_cast_does_not_change_the_pointer_value_even_if_chaining_casts_through_void_pointer
    )
  {
    static_assert (alignof (float) <= alignof (int));

    auto const i {testing::random::value<int>{}()};
    auto const i_addr {static_cast<void const*> (std::addressof (i))};
    auto const f_ptr {util::cast<float const*> (i_addr)};

    ASSERT_EQ
      ( static_cast<void const*> (std::addressof (i))
      , static_cast<void const*> (f_ptr)
      );
  }

  TEST_F (UtilCastF, pointer_unsigned_cast_documentation_example_works)
  {
    auto const x {testing::random::value<int>{}()};
    auto const xiaddr {util::cast<std::uintptr_t> (std::addressof (x))};
    auto const y {*util::cast<int const*> (xiaddr)};

    ASSERT_EQ (x, y);
  }

  TEST_F (UtilCastF, pointer_unsigned_pointer_cast_does_not_change_pointer)
  {
    auto const x {testing::random::value<int>{}()};
    auto const xaddr {std::addressof (x)};
    auto const yaddr
      { util::cast<int const*> (util::cast<std::uintptr_t> (xaddr))
      };

    ASSERT_EQ (xaddr, yaddr);
  }
}
