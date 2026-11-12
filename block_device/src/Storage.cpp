// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/Storage.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::block_device
{
  auto make_storage
    ( util::ASIO::AnyConnectable provider_connectable
    , core::storage::ID storage_id
    , core::storage::Parameter storage_parameter_chunk_description
    , core::storage::segment::ID segment_id
    , core::memory::Range range
    ) -> Storage
  {
    return Storage
      { .provider_connectable = std::move (provider_connectable)
      , .storage_id = storage_id
      , .storage_parameter_chunk_description =
          std::move (storage_parameter_chunk_description)
      , .segment_id = segment_id
      , .range = range
      };
  }
}

namespace mcs::serialization
{
  auto Implementation<mcs::block_device::Storage>::output
    ( OArchive& oa
    , mcs::block_device::Storage const& value
    ) -> OArchive&
  {
    save (oa, value.provider_connectable);
    save (oa, value.storage_id);
    save (oa, value.storage_parameter_chunk_description);
    save (oa, value.segment_id);
    save (oa, value.range);

    return oa;
  }
  auto Implementation<mcs::block_device::Storage>::input
    ( IArchive& ia
    ) -> mcs::block_device::Storage
  {
    auto _provider_connectable {load<decltype (mcs::block_device::Storage::provider_connectable)> (ia)};
    auto _storage_id {load<decltype (mcs::block_device::Storage::storage_id)> (ia)};
    auto _storage_parameter_chunk_description {load<decltype (mcs::block_device::Storage::storage_parameter_chunk_description)> (ia)};
    auto _segment_id {load<decltype (mcs::block_device::Storage::segment_id)> (ia)};
    auto _range {load<decltype (mcs::block_device::Storage::range)> (ia)};

    return mcs::block_device::make_storage
      ( std::move (_provider_connectable)
      , _storage_id
      , std::move (_storage_parameter_chunk_description)
      , _segment_id
      , _range
      );
  }
}
