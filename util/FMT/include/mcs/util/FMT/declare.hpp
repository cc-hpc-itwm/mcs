// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Declare a customization of fmt
//
// EXAMPLE
//
// namespace fmt
// {
//   template<> MCS_UTIL_FMT_DECLARE (ns::type);
//   template<typename T> MCS_UTIL_FMT_DECLARE (ns::tmpl<T>);
// }
//
#define MCS_UTIL_FMT_DECLARE(_type...)                   \
  MCS_UTIL_FMT_DECLARE_IMPL(_type)

// Declare class with format members. Allows for state in the
// formatter.
//
// EXAMPLE
//
// namespace fmt
// {
//   template<typename T>
//     MCS_UTIL_FMT_DECLARE_FORMATTER (ns::tmpl<T>)
//   {
//     MCS_UTIL_FMT_DECLARE_FORMATTER_MEMBERS (ns::tmpl<T>);
//
//   private:
//     formatter_state;
//   };
// }
//
#define MCS_UTIL_FMT_DECLARE_FORMATTER(_type...)         \
  MCS_UTIL_FMT_DECLARE_FORMATTER_IMPL(_type)

#define MCS_UTIL_FMT_DECLARE_FORMATTER_MEMBERS(_type...) \
  MCS_UTIL_FMT_DECLARE_FORMATTER_MEMBERS_IMPL (_type)

#include "detail/declare.ipp"
