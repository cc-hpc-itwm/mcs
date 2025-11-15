// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <list>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/RangesIterator.hpp>
#include <stdexcept>
#include <vector>

namespace
{
  struct Range
  {
    [[nodiscard]] constexpr explicit Range (int begin, int end)
      : _begin {begin}
      , _end {end}
    {
      if (! (_begin < _end))
      {
        throw std::invalid_argument
          { fmt::format
             ( "Range::Range ({}, {}): begin must be smaller than end"
             , _begin
             , _end
             )
          };
      }
    }

    int _begin;
    int _end;
  };

  [[nodiscard]] constexpr auto begin (Range const& range) noexcept -> int
  {
    return range._begin;
  }
  [[nodiscard]] constexpr auto end (Range const& range) noexcept -> int
  {
    return range._end;
  }

  using Ranges = std::list<Range>;
}

namespace mcs::util
{
  TEST (UtilRangesIterator, default_constructed_is_at_end)
  {
    ASSERT_FALSE (RangesIterator<Ranges>{});
  }
  TEST (UtilRangesIterator, default_constructed_equals_default_constructed)
  {
    ASSERT_TRUE (RangesIterator<Ranges>{} == RangesIterator<Ranges>{});
    ASSERT_FALSE (RangesIterator<Ranges>{} != RangesIterator<Ranges>{});
  }

  struct UtilRangesIteratorR : public testing::random::Test{};

  TEST_F (UtilRangesIteratorR, iterates_over_all_elements)
  {
    testing::random::value<int> random_value {-1000, 1000};
    auto const xs
      { std::invoke
        ( [&]
          {
            auto _xs
              { testing::random::unique_values<int>
                ( random_value
                , 2 * testing::random::value<std::size_t> {0, 10}()
                )
              };
            std::ranges::sort (_xs);

            return _xs;
          }
        )
      };
    auto const ranges
      { std::invoke
        ( [&]
          {
            Ranges _ranges;

            for (auto x {std::cbegin (xs)}; x != std::cend (xs); x += 2)
            {
              _ranges.emplace_back (*x, *std::next (x));
            }

            return _ranges;
          }
        )
      };
    auto const flattened
      { std::invoke
        ( [&]
          {
            auto _flattened {std::vector<int>{}};

            for (auto const& range : ranges)
            {
              for (auto x {begin (range)}; x != end (range); ++x)
              {
                _flattened.emplace_back (x);
              }
            }

            return _flattened;
          }
        )
      };
    auto expected {std::cbegin (flattened)};

    auto id {RangesIterator {ranges}};

    while (id && expected != std::cend (flattened))
    {
      ASSERT_EQ (*id, *expected);

      ++id;
      ++expected;
    }

    ASSERT_TRUE (!id);
    ASSERT_TRUE (expected == std::cend (flattened));
  }
}
