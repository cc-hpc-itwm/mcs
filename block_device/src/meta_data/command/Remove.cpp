// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/meta_data/command/Remove.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::meta_data::command::Remove>::output
    ( OArchive& oa
    , mcs::block_device::meta_data::command::Remove const& value
    ) -> OArchive&
  {
    save (oa, value.range);

    return oa;
  }
  auto Implementation<mcs::block_device::meta_data::command::Remove>::input
    ( IArchive& ia
    ) -> mcs::block_device::meta_data::command::Remove
  {
    auto range {load<decltype (mcs::block_device::meta_data::command::Remove::range)> (ia)};

    return mcs::block_device::meta_data::command::Remove {range};
  }
}
