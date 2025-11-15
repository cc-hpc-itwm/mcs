// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>

namespace mcs::core::storage
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    struct Segment
  {
    using SupportedStorageImplementations
      = util::type::List<StorageImplementations...>
      ;

    [[nodiscard]] Segment
      ( util::not_null<Storages<SupportedStorageImplementations>>
      , ID
      , memory::Size
      , typename StorageImplementation::Parameter::Segment::Create
      );

    [[nodiscard]] constexpr auto id() const noexcept -> segment::ID;

  private:
    segment::ID _id;
  };
}

namespace mcs::core::storage::segment
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    struct Deleter
  {
    Deleter ( util::not_null
                < Storages<util::type::List<StorageImplementations...>>
                >
            , storage::ID
            , typename StorageImplementation::Parameter::Segment::Remove
            ) noexcept;

    auto operator()
      ( Segment<StorageImplementation, StorageImplementations...>*
      ) const noexcept -> void
      ;

 private:
    util::not_null<Storages<util::type::List<StorageImplementations...>>>
      _storages;
    storage::ID _storage_id;
    typename StorageImplementation::Parameter::Segment::Remove
      _parameter_segment_remove
      ;
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
    using UniqueSegment = std::unique_ptr
      < Segment<StorageImplementation, StorageImplementations...>
      , segment::Deleter<StorageImplementation, StorageImplementations...>
      >;
}

namespace mcs::core::storage
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    [[nodiscard]] auto make_unique_segment
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
      , ID
      , memory::Size
      , typename StorageImplementation::Parameter::Segment::Create = {}
      , typename StorageImplementation::Parameter::Segment::Remove = {}
      ) -> UniqueSegment<StorageImplementation, StorageImplementations...>
    ;
}

namespace mcs::core::storage
{
  template< is_implementation StorageImplementation
          , is_implementation... StorageImplementations
          >
    requires ( util::type::List<StorageImplementations...>
                 ::template contains<StorageImplementation>()
             )
    [[nodiscard]] auto make_unique_segment
      ( Storages<util::type::List<StorageImplementations...>>*
      , ID
      , memory::Size
      , typename StorageImplementation::Parameter::Segment::Create = {}
      , typename StorageImplementation::Parameter::Segment::Remove = {}
      ) -> UniqueSegment<StorageImplementation, StorageImplementations...>
    ;
}

#include "detail/UniqueSegment.ipp"
