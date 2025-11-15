// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Define a custom hash implementation
//
// EXAMPLE:
//
//     namespace std
//     {
//       MCS_UTIL_HASH_DEFINE (x, T)
//       {
//         return std::hash<decltype (x.m)>{} (x.m);
//       }
//     }
//
#define MCS_UTIL_HASH_DEFINE(_variable, _type...)        \
  MCS_UTIL_HASH_DEFINE_IMPL(_variable, _type)

// Implement a customization of std::hash using the hash of a member.
//
// Requires:
// - std::hash<decltype (_type::member)> is defined
// - T::_member can be accessed by std::hash (use MCS_UTIL_HASH_ACCESS())
//
// EXAMPLE:
//
//   namespace std
//   {
//     MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER (_member, ns::type);
//   }
//
#define MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER(_member, _type...)       \
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER_IMPL(_member, _type)

// Implement a customization of std::hash for type wrappers.
//
// Requires:
// - std::hash<T::underlying_type> is defined
// - T::_member can be accessed by std::hash (use MCS_UTIL_HASH_ACCESS())
// - T::_member has type T::underlying_type
//
// EXAMPLE:
//
//   namespace std
//   {
//     MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_UNDERLYING_TYPE (_value, ns::type);
//   }
//
#define MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_UNDERLYING_TYPE(_member, _type...) \
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_UNDERLYING_TYPE_IMPL(_member, _type)

#include "detail/define.ipp"
