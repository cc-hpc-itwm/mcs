// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/block/Size.hpp>
#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::block_device::block::Size
  , _value
  );

namespace mcs::block_device::block
{
  Size::Error::MustNotBeZero::MustNotBeZero()
    : mcs::Error {"mcs::block_device::block::Size::MustNotBeZero"}
  {}
  Size::Error::MustNotBeZero::~MustNotBeZero() = default;

}
