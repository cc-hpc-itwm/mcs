// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/storage/Parameter.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <utility>

namespace mcs::core::storage
{
  Parameter::Parameter (std::vector<std::byte> blob)
    : _blob {blob}
  {}
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::Parameter
  , _blob
  );
