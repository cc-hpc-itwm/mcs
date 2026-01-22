// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/share_service/Chunk.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::share_service::Chunk>::output
    ( OArchive& oa
    , mcs::share_service::Chunk const& value
    ) -> OArchive&
  {
    save (oa, value.provider_connectable);
    save (oa, value.storage_id);
    save (oa, value.storage_implementation_id);
    save (oa, value.segment_id);
    save (oa, value.size);

    return oa;
  }
  auto Implementation<mcs::share_service::Chunk>::input
    ( IArchive& ia
    ) -> mcs::share_service::Chunk
  {
    auto provider_connectable {load<decltype (mcs::share_service::Chunk::provider_connectable)> (ia)};
    auto storage_id {load<decltype (mcs::share_service::Chunk::storage_id)> (ia)};
    auto storage_implementation_id {load<decltype (mcs::share_service::Chunk::storage_implementation_id)> (ia)};
    auto segment_id {load<decltype (mcs::share_service::Chunk::segment_id)> (ia)};
    auto size {load<decltype (mcs::share_service::Chunk::size)> (ia)};

    return mcs::share_service::Chunk
      { provider_connectable
      , storage_id
      , storage_implementation_id
      , segment_id
      , size
      };
  }
}
