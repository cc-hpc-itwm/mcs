// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/iov_backend/UsedStorage.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION3
  ( mcs::iov_backend::UsedStorage
  , _range
  , _segment_id
  , _storage_id
  );
