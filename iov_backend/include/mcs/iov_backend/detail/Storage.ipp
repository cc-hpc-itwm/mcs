// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_FMT_READ12
  ( "mcs::iov_backend::Storage "
  , mcs::iov_backend::Storage
  , _storage_implementation_id
  , _parameter_create
  , _storages_provider
  , _transport_provider
  , _storage_id
  , _parameter_size_max
  , _parameter_size_used
  , _parameter_segment_create
  , _parameter_segment_remove
  , _parameter_chunk_description
  , _parameter_file_read
  , _parameter_file_write
  );
