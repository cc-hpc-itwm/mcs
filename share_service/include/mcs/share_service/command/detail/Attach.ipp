// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    auto Implementation<share_service::command::Attach<Access>>::input
      ( IArchive& ia
      ) -> share_service::command::Attach<Access>
  {
    using Attach = share_service::command::Attach<Access>;

    auto chunk {load<decltype (Attach::chunk)> (ia)};
    auto parameters {load<decltype (Attach::parameters)> (ia)};

    return Attach {chunk, parameters};
  }

  template<core::chunk::is_access Access>
    auto Implementation<share_service::command::Attach<Access>>::output
      ( OArchive& oa
      , share_service::command::Attach<Access> const& attach
      ) -> OArchive&
  {
    save (oa, attach.chunk);
    save (oa, attach.parameters);

    return oa;
  }
}

namespace mcs::serialization
{
  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    auto Implementation<share_service::command::attach::Parameters<StorageImplementation>>::input
      ( IArchive& ia
      ) -> share_service::command::attach::Parameters<StorageImplementation>
  {
    using Parameters =
      share_service::command::attach::Parameters<StorageImplementation>
      ;

    auto chunk_description {load<decltype (Parameters::chunk_description)> (ia)};

    return Parameters {chunk_description};
  }

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    auto Implementation<share_service::command::attach::Parameters<StorageImplementation>>::output
      ( OArchive& oa
      , share_service::command::attach::Parameters<StorageImplementation> const& parameters
      ) -> OArchive&
  {
    save (oa, parameters.chunk_description);

    return oa;
  }
}
