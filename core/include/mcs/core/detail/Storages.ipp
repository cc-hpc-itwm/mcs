// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mutex>
#include <stdexcept>
#include <utility>

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::HeterogeneousMap<storage::ID, StorageImplementations...>
                   ::Values::template contains<StorageImplementation>()
               )
    constexpr auto Storages< util::type::List<StorageImplementations...>
                           >::implementation_id
        (
        ) noexcept -> ImplementationID
  {
    return Base::Values::template id<StorageImplementation>();
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages<util::type::List<StorageImplementations...>>::size_max
        ( typename Base::ReadAccess const& read_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::Size::Max
            parameter_size_max
        ) const -> storage::MaxSize
  {
    return std::get<StorageImplementation>
      ( Base::at (read_access_token, storage_id)
      ).size_max
          ( parameter_size_max
          )
      ;
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages<util::type::List<StorageImplementations...>>::size_used
        ( typename Base::ReadAccess const& read_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::Size::Used
            parameter_size_used
        ) const -> memory::Size
  {
    return std::get<StorageImplementation>
      ( Base::at (read_access_token, storage_id)
      ).size_used
          ( parameter_size_used
          )
      ;
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages<util::type::List<StorageImplementations...>>::segment_create
        ( typename Base::WriteAccess const& write_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::Segment::Create
            parameter_segment_create
        , memory::Size memory_size
        ) -> storage::segment::ID
  {
    return std::get<StorageImplementation>
      ( Base::at (write_access_token, storage_id)
      ).segment_create
          ( parameter_segment_create
          , memory_size
          )
      ;
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages<util::type::List<StorageImplementations...>>::segment_remove
        ( typename Base::WriteAccess const& write_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::Segment::Remove
            parameter_segment_remove
        , storage::segment::ID segment_id
        ) -> memory::Size
  {
    return std::get<StorageImplementation>
      ( Base::at (write_access_token, storage_id)
      ).segment_remove
          ( parameter_segment_remove
          , segment_id
          )
      ;
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template< storage::is_implementation StorageImplementation
            , chunk::is_access Access
            >
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages< util::type::List<StorageImplementations...>
                   >::chunk_description
        ( typename Base::ReadAccess const& read_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::Chunk::Description
            parameter_chunk_description
        , storage::segment::ID segment_id
        , memory::Range memory_range
        ) const -> chunk::Description<Access, StorageImplementations...>
  {
    return std::get<StorageImplementation>
      ( Base::at (read_access_token, storage_id)
      ).template chunk_description<Access>
          ( parameter_chunk_description
          , segment_id
          , memory_range
          )
      ;
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages<util::type::List<StorageImplementations...>>::file_read
        ( typename Base::ReadAccess const& read_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::File::Read
            parameter_file_read
        , storage::segment::ID segment_id
        , memory::Offset offset
        , std::filesystem::path path
        , memory::Range range
        ) const -> memory::Size
  {
    return std::get<StorageImplementation>
      ( Base::at (read_access_token, storage_id)
      ).file_read
          ( parameter_file_read
          , segment_id
          , offset
          , path
          , range
          )
      ;
      // \todo was this correct, e.g. to unlock while the operation is executed!?
      // , std::invoke
      //   ( [&]() -> storage::Implementation&
      //     {
      //       auto const lock {std::shared_lock (_guard)};

      //       return at (storage_id, lock);
      //     }
      //   )
      // );
  }
}

namespace mcs::core
{
  template<storage::is_implementation... StorageImplementations>
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto Storages<util::type::List<StorageImplementations...>>::file_write
        ( typename Base::ReadAccess const& read_access_token
        , storage::ID storage_id
        , typename StorageImplementation::Parameter::File::Write
            parameter_file_write
        , storage::segment::ID segment_id
        , memory::Offset offset
        , std::filesystem::path path
        , memory::Range range
        ) const -> memory::Size
  {
    return std::get<StorageImplementation>
      ( Base::at (read_access_token, storage_id)
      ).file_write
          ( parameter_file_write
          , segment_id
          , offset
          , path
          , range
          )
      ;
      // \todo was this correct, e.g. to unlock while the operation is executed!?
      // , std::invoke
      //   ( [&]() -> storage::Implementation&
      //     {
      //       auto const lock {std::shared_lock (_guard)};

      //       return at (storage_id, lock);
      //     }
      //   )
      // );
  }
}
