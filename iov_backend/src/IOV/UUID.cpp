// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/IOV_UUID.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/util/hash/define.hpp>
#include <type_traits>

namespace std
{
  MCS_UTIL_HASH_DEFINE (uuid, iov::meta::UUID)
  {
    return std::hash<iov::meta::UUID_FingerPrint>{} (uuid.str());
  }
}

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    ( oa
    , uuid
    , iov::meta::UUID
    )
  {
    save (oa, uuid.str());

    return oa;
  }

  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    ( ia
    , iov::meta::UUID
    )
  {
    auto str {load<iov::meta::UUID_FingerPrint> (ia)};

    return iov::meta::UUID {str};
  }
}
