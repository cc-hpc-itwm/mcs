// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/Error.hpp>

namespace mcs::util
{
  // Returns: divru (x, y) == q iff x <= q*y
  //
  template<std::unsigned_integral T>
    constexpr auto divru (T const&, T const&) -> T;

  struct Divru
  {
    template<std::unsigned_integral T>
      struct Error : public mcs::Error
    {
      Error (T, T) noexcept;

      constexpr auto numerator() const noexcept -> T;
      constexpr auto denominator() const noexcept -> T;

      MCS_ERROR_COPY_MOVE_DEFAULT (Error);

    private:
      T _numerator;
      T _denominator;
    };

    struct DivisionByZero : public mcs::Error
    {
      DivisionByZero();

      MCS_ERROR_COPY_MOVE_DEFAULT (DivisionByZero);
    };

    struct OverflowInIntermediateValue : public mcs::Error
    {
      OverflowInIntermediateValue();

      MCS_ERROR_COPY_MOVE_DEFAULT (OverflowInIntermediateValue);
    };
  };
}

#include "detail/divru.ipp"
