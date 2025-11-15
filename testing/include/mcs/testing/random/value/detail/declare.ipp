// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/value.hpp>

#define MCS_TESTING_RANDOM_DECLARE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE_IMPL(_type) \
  struct value<_type> : public value<_type::underlying_type>            \
  {                                                                     \
    using Base = value<_type::underlying_type>;                         \
    using Base::Base;                                                   \
                                                                        \
    auto operator()() -> _type;                                         \
  }
