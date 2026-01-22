// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/block/ID.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::block::ID>::output
    ( OArchive& oa
    , mcs::block_device::block::ID const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::block_device::block::ID>::input
    ( IArchive& ia
    ) -> mcs::block_device::block::ID
  {
    auto _value {load<decltype (mcs::block_device::block::ID::_value)> (ia)};

    return mcs::block_device::block::ID {_value};
  }
}
