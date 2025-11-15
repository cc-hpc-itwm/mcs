// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/select.hpp>
#include <span>
#include <vector>

namespace mcs::util
{
  struct UtilSelect : public testing::random::Test
  {
    using RandomSize = testing::random::value<std::size_t>;
    using RandomOffset = RandomSize;
    using RandomCount = RandomSize;
  };

  TEST_F (UtilSelect, empty_subspan_of_empty_span_can_be_selected)
  {
    auto const values {std::vector<int>{}};
    auto const span {std::span {std::cbegin (values), std::cend (values)}};

    auto const selected {select (span, 0, 0)};

    ASSERT_EQ (selected.size(), 0);
  }

  TEST_F (UtilSelect, empty_subspan_can_be_selected)
  {
    auto const values {std::vector<int> (RandomSize {0, 1000}())};
    auto const span {std::span {std::cbegin (values), std::cend (values)}};
    auto const offset {RandomSize {0, values.size()}()};

    auto const selected {select (span, offset, 0)};

    ASSERT_EQ (selected.size(), 0);
  }

  TEST_F (UtilSelect, nonempty_subspan_can_be_selected)
  {
    auto values {std::vector<int> (RandomSize {0, 1000}())};
    auto const span {std::span {std::cbegin (values), std::cend (values)}};
    auto const offset {RandomSize {0, values.size()}()};
    auto const count {RandomSize {0, values.size() - offset}()};

    auto const selected {select (span, offset, count)};

    ASSERT_EQ (selected.size(), count);

    std::ranges::generate (values, testing::random::value<int>{});

    auto expected
      { std::cbegin (values)
      + util::cast<typename std::iterator_traits<decltype (std::cbegin (values))>::difference_type> (offset)
      };

    for (auto x : selected)
    {
      ASSERT_NE (expected, std::cend (values));
      ASSERT_EQ (x, *expected++);
    }
  }

  TEST_F ( UtilSelect
         , to_select_a_subspan_that_starts_behind_the_values_throws
         )
  {
    auto values {std::vector<int> (RandomSize {0, 1000}())};
    auto const span {std::span {std::cbegin (values), std::cend (values)}};
    auto const offset {RandomSize {RandomSize::Min {values.size() + 1}}()};

    ASSERT_GT (offset, values.size());

    auto const count {RandomSize{}()};

    testing::require_exception
      ( [&]
        {
          std::ignore = select (span, offset, count);
        }
      , testing::Assert<Select::Error::OutOfRange>
        { [&] (auto& caught)
          {
            ASSERT_EQ (caught.offset(), offset);
            ASSERT_EQ (caught.count(), count);
            ASSERT_EQ (caught.size(), values.size());
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "select: offset + count > data.size(): {} + {} > {}"
                , offset
                , count
                , values.size()
                ).c_str()
              );
          }
        }
      );
  }

  TEST_F ( UtilSelect
         , to_select_a_subspan_that_ends_behind_the_values_throws
         )
  {
    auto values {std::vector<int> (RandomSize {0, 1000}())};
    auto const span {std::span {std::cbegin (values), std::cend (values)}};
    auto const offset {RandomSize {0, values.size()}()};

    ASSERT_LE (offset, values.size());

    auto const rest {values.size() - offset};
    auto const count {RandomSize {RandomSize::Min {rest + 1}}()};

    ASSERT_GT (count, rest);

    testing::require_exception
      ( [&]
        {
          std::ignore = select (span, offset, count);
        }
      , testing::Assert<Select::Error::OutOfRange>
        { [&] (auto& caught)
          {
            ASSERT_EQ (caught.offset(), offset);
            ASSERT_EQ (caught.count(), count);
            ASSERT_EQ (caught.size(), values.size());
            ASSERT_STREQ
              ( caught.what()
              , fmt::format
                ( "select: offset + count > data.size(): {} + {} > {}"
                , offset
                , count
                , values.size()
                ).c_str()
              );
          }
        }
      );
  }
}
