// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/require_semi.hpp>

#define MCS_TESTING_RANDOM_DEFINE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE_IMPL(_type, _make...) \
  auto value<_type>::operator()() -> _type                              \
  {                                                                     \
    return _make (Base::operator()());                                  \
  } MCS_UTIL_REQUIRE_SEMI()
