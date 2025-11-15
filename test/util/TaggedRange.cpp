// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/TaggedRange.hpp>
#include <vector>

namespace mcs::util
{
  using T = unsigned int;
  using Containers = ::testing::Types<std::list<T>, std::vector<T>>;
  template<class> struct UtilTaggedRangeC : testing::random::Test
  {
    testing::random::value<T> random_value;

    template<typename Container>
      auto random()
    {
      auto container {Container{}};

      std::generate_n
        ( std::back_inserter (container)
        , testing::random::value<std::size_t> {0u, 1000u}()
        , random_value
        );

      return container;
    }
  };
  TYPED_TEST_SUITE (UtilTaggedRangeC, Containers);

  TYPED_TEST (UtilTaggedRangeC, empty_range_leads_empty_tags_ordered)
  {
    using Container = TypeParam;
    auto const range {Container{}};
    auto const tagged_range {make_ordered_tagged_range (0, range)};
    ASSERT_TRUE (std::empty (tagged_range.tags()));
  }
  TYPED_TEST (UtilTaggedRangeC, empty_range_leads_empty_tags_unordered)
  {
    using Container = TypeParam;
    auto const range {Container{}};
    auto const tagged_range {make_unordered_tagged_range (0, range)};
    ASSERT_TRUE (std::empty (tagged_range.tags()));
  }

  TYPED_TEST (UtilTaggedRangeC, tags_size_equals_range_size_ordered)
  {
    using Container = TypeParam;
    auto const range {this->template random<Container>()};
    auto const tagged_range {make_ordered_tagged_range (0, range)};
    ASSERT_EQ (std::size (range), std::size (tagged_range.tags()));
  }
  TYPED_TEST (UtilTaggedRangeC, tags_size_equals_range_size_unordered)
  {
    using Container = TypeParam;
    auto const range {this->template random<Container>()};
    auto const tagged_range {make_unordered_tagged_range (0, range)};
    ASSERT_EQ (std::size (range), std::size (tagged_range.tags()));
  }

  TYPED_TEST (UtilTaggedRangeC, ordered_tags_are_in_range_order)
  {
    using Container = TypeParam;
    auto const range {this->template random<Container>()};
    auto const tagged_range {make_ordered_tagged_range (0, range)};

    for ( auto range_element {std::cbegin (range)}
        ; auto tag : tagged_range.tags()
        )
    {
      ASSERT_EQ (tagged_range.at (tag), *range_element);

      ++range_element;
    }
  }

  TYPED_TEST (UtilTaggedRangeC, unordered_tags_cover_all_range_elements)
  {
    using Container = TypeParam;
    auto const range {this->template random<Container>()};
    auto const tagged_range {make_unordered_tagged_range (0, range)};

    auto extracted {Container{}};

    for (auto tag : tagged_range.tags())
    {
      extracted.emplace_back (tagged_range.at (tag));
    }

    ASSERT_THAT (extracted, ::testing::UnorderedElementsAreArray (range));
  }
}
