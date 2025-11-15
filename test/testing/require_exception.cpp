// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <functional>
#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <mcs/Error.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/FMT/define.hpp>

namespace
{
  struct CustomError : public mcs::Error
  {
    MCS_ERROR_COPY_MOVE_DEFAULT (CustomError);
    CustomError (int value = 0)
      : mcs::Error {fmt::format ("CustomError {}", value)}
      , _value {value}
    {}
    int _value;
  };
  CustomError::~CustomError() = default;

  struct AnotherCustomError : public mcs::Error
  {
    MCS_ERROR_COPY_MOVE_DEFAULT (AnotherCustomError);
    AnotherCustomError (int value = 0)
      : mcs::Error {fmt::format ("AnotherCustomError {}", value)}
      , _value {value}
    {}
    int _value;
  };
  AnotherCustomError::~AnotherCustomError() = default;

  struct OneMoreCustomError : public mcs::Error
  {
    MCS_ERROR_COPY_MOVE_DEFAULT (OneMoreCustomError);
    OneMoreCustomError (int value = 0)
      : mcs::Error {fmt::format ("OneMoreCustomError {}", value)}
      , _value {value}
    {}
    int _value;
  };
  OneMoreCustomError::~OneMoreCustomError() = default;
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (::CustomError);
  template<> MCS_UTIL_FMT_DECLARE (::AnotherCustomError);
  template<> MCS_UTIL_FMT_DECLARE (::OneMoreCustomError);
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (context, ::CustomError)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (custom_error, context, ::CustomError)
  {
    return fmt::format_to
      ( context.out()
      , "{}"
      , std::invoke
        ( [&]
          {
            try
            {
              throw custom_error;
            }
            catch (...)
            {
              return fmt::format ("{}", std::current_exception());
            }
          }
        )
      );
  }

  MCS_UTIL_FMT_DEFINE_PARSE (context, ::AnotherCustomError)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (custom_error, context, ::AnotherCustomError)
  {
    return fmt::format_to
      ( context.out()
      , "{}"
      , std::invoke
        ( [&]
          {
            try
            {
              throw custom_error;
            }
            catch (...)
            {
              return fmt::format ("{}", std::current_exception());
            }
          }
        )
      );
  }

  MCS_UTIL_FMT_DEFINE_PARSE (context, ::OneMoreCustomError)
  {
    return context.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (custom_error, context, ::OneMoreCustomError)
  {
    return fmt::format_to
      ( context.out()
      , "{}"
      , std::invoke
        ( [&]
          {
            try
            {
              throw custom_error;
            }
            catch (...)
            {
              return fmt::format ("{}", std::current_exception());
            }
          }
        )
      );
  }
}

namespace mcs::testing
{
  namespace
  {
    struct MCSTestingRequireExceptionR : public testing::random::Test{};
  }
}

namespace mcs::testing
{
  TEST ( MCSTestingRequireException
       , failure_if_no_exception_happens
       )
  {
    EXPECT_FATAL_FAILURE
      ( require_exception ([] { return; }, testing::assert_type<Error>());
      , "Missing exception."
      );
  }

  TEST ( MCSTestingRequireException
       , failure_if_exception_happens_but_none_is_expected
       )
  {
    EXPECT_FATAL_FAILURE
      ( require_exception ([&] { throw CustomError{}; });
      , fmt::format ("Unexpected exception: {}\n", CustomError{})
      );
  }

  TEST ( MCSTestingRequireException
       , failure_if_exception_happens_but_another_type_is_expected
       )
  {
    EXPECT_FATAL_FAILURE
      ( require_exception
        ( [&] { throw CustomError{}; }
        , assert_type<AnotherCustomError>()
        );
      , fmt::format
        ( "Type mismatch.\n"
          "Expected type: {}\n"
          "Got exception: {}\n"
        , typeid (AnotherCustomError).name()
        , CustomError{}
        )
      );
  }

  TEST_F ( MCSTestingRequireExceptionR
         , expected_exception_can_be_checked_by_callback
         )
  {
    auto const value {std::invoke (random::value<int>{})};

    require_exception
      ( [value]
        {
          throw CustomError {value};
        }
      , testing::Assert<CustomError>
        { [value] (auto const& caught)
          {
            static_assert
              ( std::is_same_v<decltype (caught), CustomError const&>
              );

            ASSERT_EQ (caught._value, value);
          }
        }
      );

    ASSERT_TRUE (true);
  }

  TEST_F ( MCSTestingRequireExceptionR
         , expected_nested_exception_can_be_checked_by_callback
         )
  {
    auto random_int {random::value<int>{}};
    auto const value_outer {std::invoke (random_int)};
    auto const value_inner {std::invoke (random_int)};

    require_exception
      ( [value_inner, value_outer]
        {
          try
          {
            throw CustomError {value_inner};
          }
          catch (...)
          {
            std::throw_with_nested (AnotherCustomError {value_outer});
          }
        }
      , testing::Assert<AnotherCustomError>
        { [value_outer] (auto const& caught)
          {
            static_assert
              ( std::is_same_v<decltype (caught), AnotherCustomError const&>
              );

            ASSERT_EQ (caught._value, value_outer);
          }
        }
      , testing::Assert<CustomError>
        { [value_inner] (auto const& caught)
          {
            static_assert
              ( std::is_same_v<decltype (caught), CustomError const&>
              );

            ASSERT_EQ (caught._value, value_inner);
          }
        }
      );

    ASSERT_TRUE (true);
  }

  TEST_F ( MCSTestingRequireExceptionR
         , failure_if_nested_is_expected_but_thrown_exception_is_not_nested
         )
  {
    EXPECT_FATAL_FAILURE
      ( require_exception
        ( [&] { throw CustomError{}; }
        , assert_type<CustomError>()
        , assert_type<AnotherCustomError>()
        );
      , fmt::format
        ( "Not nested.\n"
          "Got type: {}\n"
        , typeid (CustomError).name()
        )
      );
  }

  TEST_F ( MCSTestingRequireExceptionR
         , expected_double_nested_exception_can_be_checked_by_callback
         )
  {
    auto random_int {random::value<int>{}};
    auto const value_inner {std::invoke (random_int)};
    auto const value_middle {std::invoke (random_int)};
    auto const value_outer {std::invoke (random_int)};

    require_exception
      ( [value_inner, value_middle, value_outer]
        {
          try
          {
            try
            {
              throw CustomError {value_inner};
            }
            catch (...)
            {
              std::throw_with_nested (AnotherCustomError {value_middle});
            }
          }
          catch (...)
          {
            std::throw_with_nested (OneMoreCustomError {value_outer});
          }
        }
      , testing::Assert<OneMoreCustomError>
        { [value_outer] (auto const& caught)
          {
            static_assert
              ( std::is_same_v<decltype (caught), OneMoreCustomError const&>
              );

            ASSERT_EQ (caught._value, value_outer);
          }
        }
      , testing::Assert<AnotherCustomError>
        { [value_middle] (auto const& caught)
          {
            static_assert
              ( std::is_same_v<decltype (caught), AnotherCustomError const&>
              );

            ASSERT_EQ (caught._value, value_middle);
          }
        }
      , testing::Assert<CustomError>
        { [value_inner] (auto const& caught)
          {
            static_assert
              ( std::is_same_v<decltype (caught), CustomError const&>
              );

            ASSERT_EQ (caught._value, value_inner);
          }
        }
      );

    ASSERT_TRUE (true);
  }
}
