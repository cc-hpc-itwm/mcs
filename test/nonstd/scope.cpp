// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/nonstd/scope.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/unique_values.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <utility>
#include <vector>

namespace mcs::nonstd
{
  struct ScopeR : public testing::random::Test
  {
  private:
    auto two_unique_random_int()
    {
      auto random_int {testing::random::value<int>{}};
      return testing::random::unique_values<int> (random_int, 2ul);
    }
    int _i;
    int const _j;

    ScopeR (std::vector<int> xs)
      : _i {xs.at (0)}
      , _j {xs.at (1)}
    {}

  public:
    ScopeR()
      : ScopeR {two_unique_random_int()}
    {}

    [[nodiscard]] auto assign_j_to_i() noexcept
    {
      return [&]() noexcept { return _i = _j; };
    }

    [[nodiscard]] auto assigned() const noexcept
    {
      return _i == _j;
    }
  };

  // --------------------------------------------------------- //

#define MCS_TEST_NONSTD_HAPPY_NOT_RELEASED(_fun, _expected)     \
  ASSERT_FALSE (assigned());                                    \
                                                                \
  {                                                             \
    auto const scope {make_scope_ ## _fun (assign_j_to_i())};   \
  }                                                             \
                                                                \
  _expected (assigned())


  TEST_F (ScopeR, scope_exit_is_called_on_happy_path_when_not_released)
  {
    MCS_TEST_NONSTD_HAPPY_NOT_RELEASED (exit, ASSERT_TRUE);
  }
  TEST_F (ScopeR, scope_success_is_called_on_happy_path_when_not_released)
  {
    MCS_TEST_NONSTD_HAPPY_NOT_RELEASED (success, ASSERT_TRUE);
  }
  TEST_F (ScopeR, scope_fail_is_not_called_on_happy_path_when_not_released)
  {
    MCS_TEST_NONSTD_HAPPY_NOT_RELEASED (fail, ASSERT_FALSE);
  }

#undef MCS_TEST_NONSTD_HAPPY_NOT_RELEASED

  // --------------------------------------------------------- //

#define MCS_TEST_NONSTD_ERROR_NOT_RELEASED(_fun, _expected)     \
  ASSERT_FALSE (assigned());                                    \
                                                                \
  struct Error{};                                               \
  try                                                           \
  {                                                             \
    auto const scope {make_scope_ ## _fun (assign_j_to_i())};   \
                                                                \
    throw Error{};                                              \
  }                                                             \
  catch (Error const& error)                                    \
  {                                                             \
    std::ignore = error;                                        \
  }                                                             \
                                                                \
  _expected (assigned())


  TEST_F (ScopeR, scope_exit_is_called_on_error_path_when_not_released)
  {
    MCS_TEST_NONSTD_ERROR_NOT_RELEASED (exit, ASSERT_TRUE);
  }
  TEST_F (ScopeR, scope_success_is_not_called_on_error_path_when_not_released)
  {
    MCS_TEST_NONSTD_ERROR_NOT_RELEASED (success, ASSERT_FALSE);
  }
  TEST_F (ScopeR, scope_fail_is_called_on_error_path_when_not_released)
  {
    MCS_TEST_NONSTD_ERROR_NOT_RELEASED (fail, ASSERT_TRUE);
  }

#undef MCS_TEST_NONSTD_ERROR_NOT_RELEASED

  // --------------------------------------------------------- //

#define MCS_TEST_NONSTD_HAPPY_RELEASED(_fun, _expected)         \
  ASSERT_FALSE (assigned());                                    \
                                                                \
  {                                                             \
    auto scope {make_scope_ ## _fun (assign_j_to_i())};         \
    scope.release();                                            \
  }                                                             \
                                                                \
  _expected (assigned())


  TEST_F (ScopeR, scope_exit_is_not_called_on_happy_path_when_released)
  {
    MCS_TEST_NONSTD_HAPPY_RELEASED (exit, ASSERT_FALSE);
  }
  TEST_F (ScopeR, scope_success_is_not_called_on_happy_path_when_released)
  {
    MCS_TEST_NONSTD_HAPPY_RELEASED (success, ASSERT_FALSE);
  }
  TEST_F (ScopeR, scope_fail_is_not_called_on_happy_path_when_released)
  {
    MCS_TEST_NONSTD_HAPPY_RELEASED (fail, ASSERT_FALSE);
  }

#undef MCS_TEST_NONSTD_HAPPY_RELEASED

  // --------------------------------------------------------- //

#define MCS_TEST_NONSTD_ERROR_RELEASED(_fun, _expected)         \
  ASSERT_FALSE (assigned());                                    \
                                                                \
  struct Error{};                                               \
  try                                                           \
  {                                                             \
    auto scope {make_scope_ ## _fun (assign_j_to_i())};         \
    scope.release();                                            \
                                                                \
    throw Error{};                                              \
  }                                                             \
  catch (Error const& error)                                    \
  {                                                             \
    std::ignore = error;                                        \
  }                                                             \
                                                                \
  _expected (assigned())


  TEST_F (ScopeR, scope_exit_is_not_called_on_error_path_when_released)
  {
  MCS_TEST_NONSTD_ERROR_RELEASED (exit, ASSERT_FALSE);
  }
  TEST_F (ScopeR, scope_success_is_not_called_on_error_path_when_released)
  {
  MCS_TEST_NONSTD_ERROR_RELEASED (success, ASSERT_FALSE);
  }
  TEST_F (ScopeR, scope_fail_is_not_called_on_error_path_when_released)
  {
  MCS_TEST_NONSTD_ERROR_RELEASED (fail, ASSERT_FALSE);
  }

#undef MCS_TEST_NONSTD_ERROR_RELEASED
}
