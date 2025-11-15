// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>
#include <utility>

namespace mcs::serialization
{
  template< core::chunk::is_access Access
          , core::storage::is_implementation... StorageImplementations
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , description
      , core::control::command::chunk::Description<Access, StorageImplementations...>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, storage_id);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, storage_parameter);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, segment_id);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, memory_range);

    return oa;
  }

  template< core::chunk::is_access Access
          , core::storage::is_implementation... StorageImplementations
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::control::command::chunk::Description<Access, StorageImplementations...>
      )
  {
    using Description
      = core::control::command::chunk::Description<Access, StorageImplementations...>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, storage_id, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, storage_parameter, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, segment_id, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, memory_range, Description);

    return Description
      { storage_id
      , storage_parameter
      , segment_id
      , memory_range
      };
  }
}
