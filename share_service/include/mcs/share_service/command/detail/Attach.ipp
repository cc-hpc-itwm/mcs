// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , share_service::command::Attach<Access>
      )
  {
    using Attach = share_service::command::Attach<Access>;

    MCS_SERIALIZATION_LOAD_FIELD (ia, chunk, Attach);
    MCS_SERIALIZATION_LOAD_FIELD (ia, parameters, Attach);

    return Attach {chunk, parameters};
  }

  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , attach
      , share_service::command::Attach<Access>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, attach, chunk);
    MCS_SERIALIZATION_SAVE_FIELD (oa, attach, parameters);

    return oa;
  }
}

namespace mcs::serialization
{
  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , share_service::command::attach::Parameters<StorageImplementation>
      )
  {
    using Parameters =
      share_service::command::attach::Parameters<StorageImplementation>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, chunk_description, Parameters);

    return Parameters {chunk_description};
  }

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , parameters
      , share_service::command::attach::Parameters<StorageImplementation>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, parameters, chunk_description);

    return oa;
  }
}
