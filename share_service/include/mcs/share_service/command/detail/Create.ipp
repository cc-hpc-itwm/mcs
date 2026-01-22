// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::serialization
{
  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    auto Implementation<share_service::command::create::Parameters<StorageImplementation>>::input
      ( IArchive& ia
      ) -> share_service::command::create::Parameters<StorageImplementation>
  {
    using Parameters =
      share_service::command::create::Parameters<StorageImplementation>
      ;

    auto create {load<decltype (Parameters::create)> (ia)};
    auto segment_create {load<decltype (Parameters::segment_create)> (ia)};

    return Parameters {create, segment_create};
  }

  template< share_service::is_supported_storage_implementation
              StorageImplementation
          >
    auto Implementation<share_service::command::create::Parameters<StorageImplementation>>::output
      ( OArchive& oa
      , share_service::command::create::Parameters<StorageImplementation> const& parameters
      ) -> OArchive&
  {
    save (oa, parameters.create);
    save (oa, parameters.segment_create);

    return oa;
  }
}
