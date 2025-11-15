// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Declare a custom hash implementation
//
// EXAMPLE:
//
//     namespace std
//     {
//       template<> MCS_UTIL_HASH_DECLARE (C);
//       template<typename T> MCS_UTIL_HASH_DECLARE (S<T>);
//     }
//
#define MCS_UTIL_HASH_DECLARE(_type...)                     \
  MCS_UTIL_HASH_DECLARE_IMPL(_type)

// Declare customization of std::hash using the hash of a member.
//
// Requires:
// - std::hash<decltype (_type::member)> is defined
//
// EXAMPLE:
//
//   namespace std
//   {
//     template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
//       ( _member
//       , ns::type
//       );
//     template<T> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER
//       ( _member
//       , ns::tmpl<T>
//       );
//   }
//
#define MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER(_member, _type...)  \
  MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_MEMBER_IMPL (_member, _type)

// Declare customization of std::hash for type wrappers.
//
// Requires:
// - std::hash<T::underlying_type> is defined
//
// EXAMPLE:
//
//   namespace std
//   {
//     template<> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
//       ( ns::type
//       );
//     template<T> MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
//       ( ns::tmpl<T>
//       );
//   }
//
#define MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE(_type...) \
  MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE_IMPL (_type)


#include "detail/declare.ipp"
