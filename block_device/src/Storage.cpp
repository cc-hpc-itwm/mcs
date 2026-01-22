// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/Storage.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::Storage>::output
    ( OArchive& oa
    , mcs::block_device::Storage const& value
    ) -> OArchive&
  {
    save (oa, value._provider_connectable);
    save (oa, value._storage_id);
    save (oa, value._storage_parameter_chunk_description);
    save (oa, value._segment_id);
    save (oa, value._range);

    return oa;
  }
  auto Implementation<mcs::block_device::Storage>::input
    ( IArchive& ia
    ) -> mcs::block_device::Storage
  {
    auto _provider_connectable {load<decltype (mcs::block_device::Storage::_provider_connectable)> (ia)};
    auto _storage_id {load<decltype (mcs::block_device::Storage::_storage_id)> (ia)};
    auto _storage_parameter_chunk_description {load<decltype (mcs::block_device::Storage::_storage_parameter_chunk_description)> (ia)};
    auto _segment_id {load<decltype (mcs::block_device::Storage::_segment_id)> (ia)};
    auto _range {load<decltype (mcs::block_device::Storage::_range)> (ia)};

    return mcs::block_device::Storage
      { _provider_connectable
      , _storage_id
      , _storage_parameter_chunk_description
      , _segment_id
      , _range
      };
  }
}
