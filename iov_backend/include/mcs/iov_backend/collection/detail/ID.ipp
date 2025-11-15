// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/IOV_UUID.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "mcs::iov_backend::collection::ID "
  , mcs::iov_backend::collection::ID
  , _uuid
  );
