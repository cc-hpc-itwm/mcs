// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/control/command/segment/Remove.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::control::command::segment::Remove>::output
    ( OArchive& oa
    , mcs::core::control::command::segment::Remove const& value
    ) -> OArchive&
  {
    save (oa, value.storage_id);
    save (oa, value.storage_parameter);
    save (oa, value.segment_id);

    return oa;
  }
  auto Implementation<mcs::core::control::command::segment::Remove>::input
    ( IArchive& ia
    ) -> mcs::core::control::command::segment::Remove
  {
    auto storage_id {load<decltype (mcs::core::control::command::segment::Remove::storage_id)> (ia)};
    auto storage_parameter {load<decltype (mcs::core::control::command::segment::Remove::storage_parameter)> (ia)};
    auto segment_id {load<decltype (mcs::core::control::command::segment::Remove::segment_id)> (ia)};

    return mcs::core::control::command::segment::Remove {storage_id, storage_parameter, segment_id};
  }
}
