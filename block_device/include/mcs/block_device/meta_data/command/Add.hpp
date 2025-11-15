// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/block_device/Storage.hpp>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::block_device::meta_data::command
{
  struct Add
  {
    using Response = block_device::meta_data::Blocks::AddResult;

    Storage storage;
  };
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    (block_device::meta_data::command::Add);
}
