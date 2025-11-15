// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/share_service/Chunk.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION5
  ( mcs::share_service::Chunk
  , provider_connectable
  , storage_id
  , storage_implementation_id
  , segment_id
  , size
  );
