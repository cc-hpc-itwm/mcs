// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::core::storage::MaxSize>::output
    ( OArchive& oa
    , mcs::core::storage::MaxSize const& value
    ) -> OArchive&
  {
    save (oa, value._limit);

    return oa;
  }
  auto Implementation<mcs::core::storage::MaxSize>::input
    ( IArchive& ia
    ) -> mcs::core::storage::MaxSize
  {
    auto _limit {load<decltype (mcs::core::storage::MaxSize::_limit)> (ia)};

    return mcs::core::storage::MaxSize {_limit};
  }
}
