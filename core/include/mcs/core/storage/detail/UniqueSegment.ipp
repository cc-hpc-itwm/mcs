// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <memory>
#include <utility>

namespace mcs::core::storage
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    Segment<StorageImplementation, StorageImplementations...>::Segment
      ( util::not_null<Storages<SupportedStorageImplementations>>
          storages
      , ID storage_id
      , memory::Size size
      , typename StorageImplementation::Parameter::Segment::Create
          parameter_segment_create
      )
        : _id
            { storages->template segment_create<StorageImplementation>
                ( storages->write_access()
                , storage_id
                , parameter_segment_create
                , size
                )
            }
  {}

  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    constexpr auto Segment<StorageImplementation, StorageImplementations...>::id
      (
      ) const noexcept -> segment::ID
  {
    return _id;
  }
}

namespace mcs::core::storage::segment
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    Deleter<StorageImplementation, StorageImplementations...>::Deleter
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      , storage::ID storage_id
      , typename StorageImplementation::Parameter::Segment::Remove
          parameter_segment_remove
      ) noexcept
        : _storages {storages}
        , _storage_id {storage_id}
        , _parameter_segment_remove {parameter_segment_remove}
  {}

  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    auto Deleter<StorageImplementation, StorageImplementations...>::operator()
      ( Segment<StorageImplementation, StorageImplementations...>* segment
      ) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "mcs::core::storage::segment::Deleter"
      , [&]
        {
          _storages->template segment_remove<StorageImplementation>
            ( _storages->write_access()
            , _storage_id
            , _parameter_segment_remove
            , segment->id()
            );

          std::default_delete
            < Segment<StorageImplementation, StorageImplementations...>
            >{} (segment);
        }
      );
  }
}


namespace mcs::core::storage
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    auto make_unique_segment
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      , ID storage_id
      , memory::Size size
      , typename StorageImplementation::Parameter::Segment::Create
          parameter_segment_create
      , typename StorageImplementation::Parameter::Segment::Remove
          parameter_segment_remove
      ) -> UniqueSegment<StorageImplementation, StorageImplementations...>
  {
    return UniqueSegment<StorageImplementation, StorageImplementations...>
      { new Segment<StorageImplementation, StorageImplementations...>
          { storages
          , storage_id
          , size
          , parameter_segment_create
          }
      , segment::Deleter<StorageImplementation, StorageImplementations...>
          { storages
          , storage_id
          , parameter_segment_remove
          }
      };
  }

  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    auto make_unique_segment
      ( Storages<util::type::List<StorageImplementations...>>* storages
      , ID storage_id
      , memory::Size size
      , typename StorageImplementation::Parameter::Segment::Create
          parameter_segment_create
      , typename StorageImplementation::Parameter::Segment::Remove
          parameter_segment_remove
      ) -> UniqueSegment<StorageImplementation, StorageImplementations...>
  {
    return make_unique_segment<StorageImplementation, StorageImplementations...>
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          {storages}
      , storage_id
      , size
      , parameter_segment_create
      , parameter_segment_remove
      );
  }
}
