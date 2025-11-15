// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <iterator>
#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <stdexcept>

namespace mcs::testing::random
{
#define MCS_TESTING_RANDOM_ONE_OF_IMPL                                  \
  auto n {std::ranges::ssize (range)};                                  \
                                                                        \
  if (n == 0)                                                           \
  {                                                                     \
    throw std::logic_error {"one_of for empty range"};                  \
  }                                                                     \
                                                                        \
  using I = decltype (n);                                               \
  using Index = value<I>;                                               \
  using AtLeast = typename Index::Min;                                  \
  using AtMost = typename Index::Max;                                   \
  auto random_index {Index {AtLeast {I {0}}, AtMost {n - I {1}}}};      \
                                                                        \
  return *std::next (std::ranges::begin (range), random_index())

  template<typename Range>
    auto one_of
      ( Range const& range
      ) -> decltype (*std::ranges::begin (range)) const&
  {
    MCS_TESTING_RANDOM_ONE_OF_IMPL;
  }

  template<typename Range>
    auto one_of
      ( Range& range
      ) -> decltype (*std::ranges::begin (range))&
  {
    MCS_TESTING_RANDOM_ONE_OF_IMPL;
  }

#undef MCS_TESTING_RANDOM_ONE_OF_IMPL
}
