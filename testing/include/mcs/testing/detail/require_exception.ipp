// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/util/FMT/STD/exception.hpp>
#include <typeinfo>
#include <utility>

namespace mcs::testing
{
  namespace detail
  {
    template<typename...> struct Nested;

    template<> struct Nested<>
    {
      auto operator() (std::exception_ptr error) const noexcept -> void
      {
        FAIL() << fmt::format ("Unexpected exception: {}\n", error);
      }
    };

    template<typename Expected>
      struct Nested<Expected>
    {
      constexpr explicit Nested
        ( testing::Assert<Expected> assert_
        )
          : _assert {assert_}
      {}

      auto operator() (std::exception_ptr error) const noexcept -> void
      {
        try
        {
          std::rethrow_exception (error);
        }
        catch (Expected const& exception)
        {
          _assert (exception);

          return;
        }
        catch (...)
        {
          FAIL() << fmt::format
            ( "Type mismatch.\n"
              "Expected type: {}\n"
              "Got exception: {}\n"
            , typeid (Expected).name()
            , std::current_exception()
            )
            ;
        }

        __builtin_unreachable();
      }

    private:
      testing::Assert<Expected> _assert;
    };

    template<typename Expected, typename... Expecteds>
      struct Nested<Expected, Expecteds...> : Nested<Expecteds...>
    {
      constexpr explicit Nested
        ( testing::Assert<Expected> assert_
        , testing::Assert<Expecteds>... asserts_
        )
          : Nested<Expecteds...> {asserts_...}
          , _assert {assert_}
      {}

      auto operator() (std::exception_ptr error) const noexcept -> void
      {
        try
        {
          std::rethrow_exception (error);
        }
        catch (Expected const& exception)
        {
          _assert (exception);

          try
          {
            std::rethrow_if_nested (exception);
          }
          catch (...)
          {
            return Nested<Expecteds...>::operator() (std::current_exception());
          }

          FAIL() << fmt::format
            ( "Not nested.\n"
              "Got type: {}\n"
            , typeid (Expected).name()
            )
            ;
        }
        catch (...)
        {
          FAIL() << fmt::format
            ( "Type mismatch.\n"
              "Expected type: {}\n"
              "Got exception: {}\n"
            , typeid (Expected).name()
            , std::current_exception()
            )
            ;
        }

        __builtin_unreachable();
      }

    private:
      testing::Assert<Expected> _assert;
    };
  }

  template<typename Expected>
    constexpr auto assert_type_or_derived_type()
  {
    return Assert<Expected> ([] (Expected const&) noexcept {});
  }

  template<typename Expected>
    constexpr auto assert_type()
  {
    return Assert<Expected>
      ( [] (Expected const& caught) noexcept
        {
          ASSERT_EQ (typeid (caught), typeid (Expected))
            << fmt::format
               ( "Caught an exception of type '{}'"
                 " which is derived from the expected type '{}'"
               , typeid (caught).name()
               , typeid (Expected).name()
               )
            ;
        }
      );
  }

  template<typename Expected, typename... Args>
    constexpr auto assert_type_and_what (Args const&... args)
  {
    return Assert<Expected>
      { [what = std::string {args...}]
          ( Expected const& caught
          ) noexcept
        {
          ASSERT_STREQ (what.c_str(), caught.what());
        }
      };
  }

  template<typename Operation, typename... Expected>
    auto require_exception
      ( Operation&& operation
      , Assert<Expected>... asserts
      ) -> void
  {
    try
    {
      std::forward<Operation> (operation)();
    }
    catch (...)
    {
      return detail::Nested<Expected...> {asserts...}
        . operator() (std::current_exception())
        ;
    }

    FAIL() << "Missing exception.";
  }
}
