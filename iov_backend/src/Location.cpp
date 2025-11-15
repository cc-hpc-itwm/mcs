// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/iov_backend/Location.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION7
  ( mcs::iov_backend::Location
  , _range
  , _storages_provider
  , _storage_implementation_id
  , _parameter_file_read
  , _parameter_file_write
  , _transport_provider
  , _address
  );
