// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>
#include <utility>

namespace mcs::serialization
{
  template< core::chunk::is_access Access
          , core::storage::is_implementation... StorageImplementations
          >
    auto Implementation<core::control::command::chunk::Description<Access, StorageImplementations...>>::output
      ( OArchive& oa
      , core::control::command::chunk::Description<Access, StorageImplementations...> const& description
      ) -> OArchive&
  {
    save (oa, description.storage_id);
    save (oa, description.storage_parameter);
    save (oa, description.segment_id);
    save (oa, description.memory_range);

    return oa;
  }

  template< core::chunk::is_access Access
          , core::storage::is_implementation... StorageImplementations
          >
    auto Implementation<core::control::command::chunk::Description<Access, StorageImplementations...>>::input
      ( IArchive& ia
      ) -> core::control::command::chunk::Description<Access, StorageImplementations...>
  {
    using Description
      = core::control::command::chunk::Description<Access, StorageImplementations...>
      ;

    auto storage_id {load<decltype (Description::storage_id)> (ia)};
    auto storage_parameter {load<decltype (Description::storage_parameter)> (ia)};
    auto segment_id {load<decltype (Description::segment_id)> (ia)};
    auto memory_range {load<decltype (Description::memory_range)> (ia)};

    return Description
      { storage_id
      , storage_parameter
      , segment_id
      , memory_range
      };
  }
}
