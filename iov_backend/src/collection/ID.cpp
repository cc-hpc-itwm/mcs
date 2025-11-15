// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/IOV_UUID.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/util/hash/define.hpp>
#include <mcs/util/tuplish/define.hpp>

namespace mcs::iov_backend::collection
{
  ID::ID ( std::string uuid
         )
    : _uuid {uuid}
  {}
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::iov_backend::collection::ID
  , _uuid
  );

namespace std
{
  MCS_UTIL_HASH_DEFINE (id, mcs::iov_backend::collection::ID)
  {
    return std::hash<decltype (id._uuid)>{} (id._uuid);
  }
}
