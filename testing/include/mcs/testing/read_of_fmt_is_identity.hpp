// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <fmt/base.h>
#include <functional>
#include <mcs/util/read/Concepts.hpp>
#include <string>

namespace mcs::testing
{
  template<typename F, typename T>
    concept is_formatter = requires (F&& formatter, T const& x)
    {
      { std::invoke (std::forward<F> (formatter), x)
      } -> std::convertible_to<std::string>
      ;
    }
    ;

  template<typename T>
    concept read_from_custom_formatter_identity_testable =
       util::read::is_readable<T>
    && std::equality_comparable<T>
    ;

  template<typename Formatter, read_from_custom_formatter_identity_testable T>
    requires (is_formatter<Formatter, T>)
    constexpr auto read_from_custom_formatter_is_identity
      ( Formatter&&
      , T const&
      )
      ;

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
