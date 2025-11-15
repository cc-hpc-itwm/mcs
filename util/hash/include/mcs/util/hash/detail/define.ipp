// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/require_semi.hpp>

#define MCS_UTIL_HASH_DEFINE_IMPL(_variable, _type...)                  \
  auto hash<_type>::operator()                                          \
    ( _type const& _variable                                            \
    ) const noexcept -> size_t

#define MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER_IMPL(_member, _type...) \
  MCS_UTIL_HASH_DEFINE (x, _type)                                       \
  {                                                                     \
    return _hash (x._member);                                           \
  } MCS_UTIL_REQUIRE_SEMI()

#define MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_UNDERLYING_TYPE_IMPL(_member, _type...) \
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_MEMBER_IMPL (_member, _type)
