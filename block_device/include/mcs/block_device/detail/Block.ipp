// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::block_device
{
  template<core::storage::is_implementation... StorageImplementations>
    Block<util::type::List<StorageImplementations...>>::Block
      ( util::not_null<core::Storages<SupportedStorageImplementations>> storages
      , core::storage::ID storage_id
      , core::storage::Parameter storage_parameter_chunk_description
      , core::storage::segment::ID segment_id
      , core::memory::Range range
      )
        : _storages {storages}
        , _storage_id {storage_id}
        , _storage_parameter_chunk_description
            {storage_parameter_chunk_description}
        , _segment_id {segment_id}
        , _range {range}
  {}

  template<core::storage::is_implementation... StorageImplementations>
    auto Block<util::type::List<StorageImplementations...>>::address
      (
      ) const -> core::transport::Address
  {
    return core::transport::Address
      { _storage_id
      , _storage_parameter_chunk_description
      , _segment_id
      , begin (_range)
      };
  }

  template<core::storage::is_implementation... StorageImplementations>
    template<core::chunk::is_access Access>
      auto Block<util::type::List<StorageImplementations...>>::chunk
        (
        ) const -> CoreChunk<Access>
  {
    return core::make_chunk<Access>
      ( _storages
      , _storage_id
      , _storage_parameter_chunk_description
      , _segment_id
      , _range
      );
  }
}
