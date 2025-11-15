// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <list>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/one_of.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace mcs::testing
{
  TEST (OneOf, empty_range_throws_temporary)
  {
    require_exception
      ( []
        {
          std::ignore = random::one_of (std::vector<int>{});
        }
      , assert_type_and_what<std::logic_error> ("one_of for empty range")
      );
  }

  TEST (OneOf, empty_range_throws_const_ref)
  {
    require_exception
      ( []
        {
          auto const xs {std::vector<int>{}};
          std::ignore = random::one_of (xs);
        }
      , assert_type_and_what<std::logic_error> ("one_of for empty range")
      );
  }

  TEST (OneOf, empty_range_throws_ref)
  {
    require_exception
      ( []
        {
          auto xs {std::vector<int>{}};
          std::ignore = random::one_of (xs);
        }
      , assert_type_and_what<std::logic_error> ("one_of for empty range")
      );
  }

  TEST (OneOf, selects_one_of_the_elements_temporary)
  {
    int const value {random::one_of (std::vector<int> ({1, 2, 3}))};

    ASSERT_GT (value, 0);
    ASSERT_LT (value, 4);
  }

  struct OneOfF : public random::Test
  {
    random::value<std::size_t> _random_size {1, 1000};
    random::value<int> _random_value;
    std::vector<int> _random_values
      {random::unique_values<int> (_random_value, _random_size())};
  };

  TEST_F (OneOfF, selects_one_of_the_elements_const_ref)
  {
    auto const values {_random_values};
    auto const value {random::one_of (values)};

    ASSERT_TRUE
      (  std::ranges::find (_random_values, value)
      != std::ranges::end (_random_values)
      );
  }

  TEST_F (OneOfF, selects_one_of_the_elements_ref)
  {
    auto const value {random::one_of (_random_values)};

    ASSERT_TRUE
      (  std::ranges::find (_random_values, value)
      != std::ranges::end (_random_values)
      );
  }

  struct NotMoveable
  {
    NotMoveable (NotMoveable const&) = delete;
    NotMoveable (NotMoveable&&) = delete;
    auto operator= (NotMoveable const&) -> NotMoveable& = delete;
    auto operator= (NotMoveable&&) -> NotMoveable& = delete;
    ~NotMoveable() = default;

    constexpr NotMoveable (int i) noexcept : _i {i} {}

    int _i;
  };

  TEST_F (OneOfF, non_moveable_allows_to_select_a_reference)
  {
    std::list<NotMoveable> xs;

    for (auto value : _random_values)
    {
      xs.emplace_back (value);
    }

    auto& x {random::one_of (xs)};

    ASSERT_TRUE
      (  std::ranges::find (_random_values, x._i)
      != std::ranges::end (_random_values)
      );
  }
}
