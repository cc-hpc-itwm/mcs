// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::serialization
{
  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , share_service::command::create::Parameters<StorageImplementation>
      )
  {
    using Parameters =
      share_service::command::create::Parameters<StorageImplementation>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, create, Parameters);
    MCS_SERIALIZATION_LOAD_FIELD (ia, segment_create, Parameters);

    return Parameters {create, segment_create};
  }

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , parameters
      , share_service::command::create::Parameters<StorageImplementation>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, parameters, create);
    MCS_SERIALIZATION_SAVE_FIELD (oa, parameters, segment_create);

    return oa;
  }
}
