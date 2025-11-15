// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>

namespace mcs::testing
{
  // Custom assertion.
  //
  template<typename T>
    using Assert = std::function<void (T const&)>;

  // Assert matching type. Does not accept types that are derived from T.
  //
  template<typename T>
    constexpr auto assert_type();

  // Assert matching type or type derived from T.
  //
  template<typename T>
    constexpr auto assert_type_or_derived_type();

  // Assert matching type and what()-message.
  //
  template<typename Expected, typename... Args>
    constexpr auto assert_type_and_what (Args const&...);

  // Require an operation to throw (nested) exception(s). Use specific
  // assertions for individual exceptions.
  //
  // EXAMPLES:
  //
  // - (1) Require an exception with specific type and message:
  //
  //     mcs::testing::require_exception
  //       ( [] { f(); }
  //       , mcs::testing::assert_type_and_what<std::runtime_error> ("Beep")
  //       );
  //
  // - (2) Require an exception with a custom assertion:
  //
  //     mcs::testing::require_exception
  //       ( [] { f(); }
  //       , mcs::testing::Assert<Ex>
  //           { [] (auto const& caught)
  //             {
  //               // the assertion is only ever called with caught
  //               // exceptions of type Ex
  //               static_assert
  //                 (std::is_same_v<std::decay_t<decltype (caught)>, Ex>);
  //
  //               ASSERT... (caught...);
  //             }
  //           }
  //       );
  //
  // - (3) Example (1) is equivalent to
  //
  //     mcs::testing::require_exception
  //       ( [] { f(); }
  //       , mcs::testing::Assert<std::runtime_error>
  //           { [] (auto const& caught)
  //             {
  //               ASSERT_STREQ (caught.what(), "Beep");
  //             }
  //           }
  //       );
  //
  // - (4) Nested exceptions are specified from outer to inner:
  //
  //     mcs::testing::require_exception
  //       ( [] { f(); }
  //       , mcs::testing::assert_type<Outer>()
  //       , mcs::testing::assert_type<Inner>()
  //       );
  //
  template<typename Operation, typename... Expected>
    auto require_exception (Operation&&, Assert<Expected>...) -> void;
}

#include "detail/require_exception.ipp"
