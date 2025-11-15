// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/Address.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION4
  ( mcs::core::transport::Address
  , storage_id
  , storage_parameter_chunk_description
  , segment_id
  , offset
  );
