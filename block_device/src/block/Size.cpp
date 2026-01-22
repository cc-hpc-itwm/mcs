// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/block/Size.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::block::Size>::output
    ( OArchive& oa
    , mcs::block_device::block::Size const& value
    ) -> OArchive&
  {
    save (oa, value._value);

    return oa;
  }
  auto Implementation<mcs::block_device::block::Size>::input
    ( IArchive& ia
    ) -> mcs::block_device::block::Size
  {
    auto _value {load<decltype (mcs::block_device::block::Size::_value)> (ia)};

    return mcs::block_device::block::Size {_value};
  }
}

namespace mcs::block_device::block
{
  Size::Error::MustNotBeZero::MustNotBeZero()
    : mcs::Error {"mcs::block_device::block::Size::MustNotBeZero"}
  {}
  Size::Error::MustNotBeZero::~MustNotBeZero() = default;

}
