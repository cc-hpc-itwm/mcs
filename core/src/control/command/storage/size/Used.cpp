// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/storage/size/Used.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::storage::size::Used>::output
    ( OArchive& oa
    , mcs::core::control::command::storage::size::Used const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);
    save (oa, value.storage_parameter);

    return oa;
  }
  auto Implementation<mcs::core::control::command::storage::size::Used>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::storage::size::Used
  {
    auto storage_id {load<decltype (mcs::core::control::command::storage::size::Used::storage_id)> (ia)};
    auto storage_parameter {load<decltype (mcs::core::control::command::storage::size::Used::storage_parameter)> (ia)};

    return mcs::core::control::command::storage::size::Used {storage_id, storage_parameter};
  }
}
