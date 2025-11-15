// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/execute_and_die_on_exception.hpp>
#include <memory>
#include <utility>

namespace mcs::core
{
  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    Storage<StorageImplementation, StorageImplementations...>::Storage
      ( util::not_null<Storages<SupportedStorageImplementations>> storages
      , typename StorageImplementation::Parameter::Create parameter_create
      )
    : _id
      { storages->template create<StorageImplementation>
          ( storages->write_access()
          , parameter_create
          )
      }
  {}

  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    constexpr auto Storage<StorageImplementation, StorageImplementations...>::id
      (
      ) const noexcept -> storage::ID
  {
    return _id;
  }

  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    constexpr auto Storage<StorageImplementation, StorageImplementations...>::implementation_id
      (
      ) const noexcept -> ImplementationID
  {
    return Storages<util::type::List<StorageImplementations...>>
      ::template implementation_id<StorageImplementation>()
      ;
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
    constexpr Deleter<StorageImplementation, StorageImplementations...>::Deleter
      ( util::not_null<Storages<SupportedStorageImplementations>> storages
      ) noexcept
        : _storages {storages}
  {}

  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    auto Deleter<StorageImplementation, StorageImplementations...>::operator()
      ( Storage<StorageImplementation, StorageImplementations...>* storage
      ) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "mcs::core::storage::Deleter"
      , [&]
        {
          _storages->remove (_storages->write_access(), storage->id());

          std::default_delete
            < Storage<StorageImplementation, StorageImplementations...>
            >{} (storage);
        }
      );
  }
}

namespace mcs::core
{
  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    auto make_unique_storage
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      , typename StorageImplementation::Parameter::Create parameter_create
      ) -> UniqueStorage<StorageImplementation, StorageImplementations...>
  {
    return UniqueStorage<StorageImplementation, StorageImplementations...>
      { new Storage<StorageImplementation, StorageImplementations...>
        { storages
        , parameter_create
        }
      , storage::Deleter<StorageImplementation, StorageImplementations...>
        { storages
        }
      };
  }

  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    auto make_unique_storage
      ( Storages<util::type::List<StorageImplementations...>>* storages
      , typename StorageImplementation::Parameter::Create parameter_create
      ) -> UniqueStorage<StorageImplementation, StorageImplementations...>
  {
    return make_unique_storage<StorageImplementation, StorageImplementations...>
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          {storages}
      , parameter_create
      );
  }
}
