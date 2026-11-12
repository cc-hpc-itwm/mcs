// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/storage/size/Used.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::core::control::command::storage::size
{
  auto make_used
    ( mcs::core::storage::ID storage_id
    , mcs::core::storage::Parameter storage_parameter
    ) -> Used
  {
    return Used
      { .storage_id = storage_id
      , .storage_parameter = std::move (storage_parameter)
      };
  }
}

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

    return mcs::core::control::command::storage::size::make_used
      ( storage_id
      , std::move (storage_parameter)
      );
  }
}
