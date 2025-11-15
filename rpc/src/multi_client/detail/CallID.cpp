// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/multi_client/detail/CallID.hpp>
#include <mcs/util/hash/define.hpp>

namespace std
{
  MCS_UTIL_HASH_DEFINE_VIA_HASH_OF_UNDERLYING_TYPE
    ( _value
    , mcs::rpc::multi_client::detail::CallID
    )
    ;
}
