// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/memory/Size.hpp>
#include <mcs/util/hash/define.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::memory::Size
  , _value
  );

namespace std
{
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_UNDERLYING_TYPE
    ( _value
    , mcs::core::memory::Size
    );
}
