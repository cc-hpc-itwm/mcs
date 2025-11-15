// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/iov_backend/command/Locations.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::iov_backend::command::Locations
  , _collection_id
  , _range
  );
