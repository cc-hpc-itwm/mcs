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

      ~Error() override;
      Error (Error const&) = default;
      Error (Error&&) noexcept = default;
      auto operator= (Error const&) -> Error& = default;
      auto operator= (Error&&) noexcept  -> Error& = default;

    private:
      T _numerator;
      T _denominator;
    };

    struct DivisionByZero : public mcs::Error
    {
      DivisionByZero();

      ~DivisionByZero() override;
      DivisionByZero (DivisionByZero const&) = default;
      DivisionByZero (DivisionByZero&&) noexcept = default;
      auto operator= (DivisionByZero const&) -> DivisionByZero& = default;
      auto operator= (DivisionByZero&&) noexcept  -> DivisionByZero& = default;
    };

    struct OverflowInIntermediateValue : public mcs::Error
    {
      OverflowInIntermediateValue();

      ~OverflowInIntermediateValue() override;
      OverflowInIntermediateValue (OverflowInIntermediateValue const&) = default;
      OverflowInIntermediateValue (OverflowInIntermediateValue&&) noexcept = default;
      auto operator= (OverflowInIntermediateValue const&) -> OverflowInIntermediateValue& = default;
      auto operator= (OverflowInIntermediateValue&&) noexcept  -> OverflowInIntermediateValue& = default;
    };
  };
}

#include "detail/divru.ipp"
