// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Enables "auto" in member declarations.
//
// EXAMPLE:
//
//     template<typename> struct F;
//
//     template<> struct F<int>
//     {
//       [[nodiscard]] constexpr auto operator() (int i) const noexcept
//       {
//         return 2 * i;
//       }
//     };
//     template<> struct F<long>
//     {
//       [[nodiscard]] constexpr auto operator() (long) const noexcept
//       {
//         return "beep";
//       }
//     };
//
//     template<typename T>
//       struct S
//     {
//       T _x;
//       MCS_UTIL_MEMBER_AUTO (_y, std::invoke (F<decltype (_x)>{}, _x));
//     };
//     static_assert (std::is_same_v<decltype (S<int>::_y), int>);
//     static_assert (std::is_same_v<decltype (S<long>::_y), char const*>);
//
#define MCS_UTIL_MEMBER_AUTO(_name, _expr...)   \
  MCS_UTIL_MEMBER_AUTO_IMPL (_name, _expr)

#include "detail/member_AUTO.ipp"
