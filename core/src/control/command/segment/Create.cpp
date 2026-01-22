// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/segment/Create.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::segment::Create>::output
    ( OArchive& oa
    , mcs::core::control::command::segment::Create const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);
    save (oa, value.storage_parameter);
    save (oa, value.memory_size);

    return oa;
  }
  auto Implementation<mcs::core::control::command::segment::Create>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::segment::Create
  {
    auto storage_id {load<decltype (mcs::core::control::command::segment::Create::storage_id)> (ia)};
    auto storage_parameter {load<decltype (mcs::core::control::command::segment::Create::storage_parameter)> (ia)};
    auto memory_size {load<decltype (mcs::core::control::command::segment::Create::memory_size)> (ia)};

    return mcs::core::control::command::segment::Create {storage_id, storage_parameter, memory_size};
  }
}
