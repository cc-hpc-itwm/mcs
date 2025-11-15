// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <fmt/base.h>
#include <mcs/util/read/Concepts.hpp>

namespace mcs::testing
{
  template<typename T>
    concept read_fmt_identity_testable =
       fmt::formattable<T>
    && util::read::is_readable<T>
    && std::equality_comparable<T>
    ;

  template<read_fmt_identity_testable T>
    constexpr auto read_of_fmt_is_identity (T const&);

  template<typename T>
    concept read_fmt_identity_testable_with_custom_checker =
       fmt::formattable<T>
    && util::read::is_readable<T>
    ;

  template<typename T, typename Checker>
    concept is_checker = requires ( T const& read_value
                                  , Checker&& check
                                  )
    {
      {check (read_value)} -> std::convertible_to<bool>;
    };

  template< read_fmt_identity_testable_with_custom_checker T
          , typename Checker
          >
    requires (is_checker<T, Checker>)
    constexpr auto read_of_fmt_is_identity (T const&, Checker&&);
}

#include "detail/read_of_fmt_is_identity.ipp"
