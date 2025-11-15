// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/Storage.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION5
  ( mcs::block_device::Storage
  , _provider_connectable
  , _storage_id
  , _storage_parameter_chunk_description
  , _segment_id
  , _range
  );
