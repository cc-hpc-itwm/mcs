// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/storage/Remove.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::storage::Remove>::output
    ( OArchive& oa
    , mcs::core::control::command::storage::Remove const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);

    return oa;
  }
  auto Implementation<mcs::core::control::command::storage::Remove>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::storage::Remove
  {
    auto storage_id {load<decltype (mcs::core::control::command::storage::Remove::storage_id)> (ia)};

    return mcs::core::control::command::storage::Remove {storage_id};
  }
}
