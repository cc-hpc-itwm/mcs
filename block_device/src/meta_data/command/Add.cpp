// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/meta_data/command/Add.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::meta_data::command::Add>::output
    ( OArchive& oa
    , mcs::block_device::meta_data::command::Add const& value
    ) -> OArchive&
  {
    save (oa, value.storage);

    return oa;
  }
  auto Implementation<mcs::block_device::meta_data::command::Add>::input
    ( IArchive& ia
    ) -> mcs::block_device::meta_data::command::Add
  {
    auto storage {load<decltype (mcs::block_device::meta_data::command::Add::storage)> (ia)};

    return mcs::block_device::meta_data::command::Add {storage};
  }
}
