// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <limits>
#include <mcs/config.hpp>

namespace mcs::util
{
  template<std::unsigned_integral T>
    constexpr auto divru (T const& x, T const& y) -> T
  try
  {
    if (y == T {0})
    {
      throw Divru::DivisionByZero{};
    }

    if (y == T {1})
    {
      return x;
    }

    if (x > std::numeric_limits<T>::max() - (y - T {1}))
    {
      throw Divru::OverflowInIntermediateValue{};
    }

    return (x + (y - T {1})) / y;
  }
  catch (...)
  {
    std::throw_with_nested (Divru::Error {x, y});
  }
}

namespace mcs::util
{
  template<std::unsigned_integral T>
    Divru::Error<T>::Error (T numerator, T denominator) noexcept
      : mcs::Error {fmt::format ("divru ({}, {})", numerator, denominator)}
      , _numerator {numerator}
      , _denominator {denominator}
  {}
#if defined (MCS_CONFIG_CLANG_TEMPLATED_DTOR_PARSER_IS_BROKEN)
  template<std::unsigned_integral T>
    Divru::Error<T>::~Error<T>() = default;
#else
  template<std::unsigned_integral T>
    Divru::Error<T>::~Error() = default;
#endif

  template<std::unsigned_integral T>
    constexpr auto Divru::Error<T>::numerator() const noexcept -> T
  {
    return _numerator;
  }
  template<std::unsigned_integral T>
    constexpr auto Divru::Error<T>::denominator() const noexcept -> T
  {
    return _denominator;
  }
}
