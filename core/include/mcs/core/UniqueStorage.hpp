// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>

namespace mcs::core
{
  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    struct Storage
  {
    using SupportedStorageImplementations
      = util::type::List<StorageImplementations...>
      ;

    explicit Storage
      ( util::not_null<Storages<SupportedStorageImplementations>>
      , typename StorageImplementation::Parameter::Create
      );

    [[nodiscard]] constexpr auto id() const noexcept -> storage::ID;

    using ImplementationID
      = typename Storages<SupportedStorageImplementations>::ImplementationID
      ;

    [[nodiscard]] constexpr auto implementation_id
      (
      ) const noexcept -> ImplementationID
      ;

  private:
    storage::ID _id;
  };
}

namespace mcs::core::storage
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    struct Deleter
  {
    using SupportedStorageImplementations
      = util::type::List<StorageImplementations...>
      ;

    constexpr explicit Deleter
      ( util::not_null<Storages<SupportedStorageImplementations>>
      ) noexcept
      ;
    auto operator()
      ( Storage<StorageImplementation, StorageImplementations...>*
      ) const noexcept -> void
      ;

  private:
    util::not_null<Storages<SupportedStorageImplementations>> _storages;
  };
}

namespace mcs::core
{
  template< storage::is_implementation StorageImplementation
          , storage::is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    using UniqueStorage = std::unique_ptr
      < Storage<StorageImplementation, StorageImplementations...>
      , storage::Deleter<StorageImplementation, StorageImplementations...>
      >
      ;
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
      , typename StorageImplementation::Parameter::Create
      ) -> UniqueStorage<StorageImplementation, StorageImplementations...>
    ;
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
      ( Storages<util::type::List<StorageImplementations...>>*
      , typename StorageImplementation::Parameter::Create
      ) -> UniqueStorage<StorageImplementation, StorageImplementations...>
    ;
}

#include "detail/UniqueStorage.ipp"
