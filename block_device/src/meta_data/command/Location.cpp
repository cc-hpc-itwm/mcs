// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/meta_data/command/Location.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::meta_data::command::Location>::output
    ( OArchive& oa
    , mcs::block_device::meta_data::command::Location const& value
    ) -> OArchive&
  {
    save (oa, value.id);

    return oa;
  }
  auto Implementation<mcs::block_device::meta_data::command::Location>::input
    ( IArchive& ia
    ) -> mcs::block_device::meta_data::command::Location
  {
    auto id {load<decltype (mcs::block_device::meta_data::command::Location::id)> (ia)};

    return mcs::block_device::meta_data::command::Location {id};
  }
}
