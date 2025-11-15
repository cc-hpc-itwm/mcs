// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/core/chunk/Description.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/util/HeterogeneousMap.hpp>

namespace mcs::core
{
  template<typename StorageImplementations> struct Storages;

  template<storage::is_implementation... StorageImplementations>
    struct Storages<util::type::List<StorageImplementations...>>
      : public util::HeterogeneousMap<storage::ID, StorageImplementations...>
  {
    using Base = util::HeterogeneousMap<storage::ID, StorageImplementations...>;

    using Base::create;
    using Base::remove;
    using Base::read_access;
    using Base::write_access;

    using ImplementationID = typename Base::Values::ID;

    template<storage::is_implementation StorageImplementation>
      requires ( util::HeterogeneousMap<storage::ID, StorageImplementations...>
                   ::Values::template contains<StorageImplementation>()
               )
      [[nodiscard]] static constexpr auto implementation_id
        (
        ) noexcept -> ImplementationID
      ;

    // Typed interface:
    // Pre: The storage at id has the type StorageImplementation.
    //
    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      [[nodiscard]] auto size_max
        ( typename Base::ReadAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::Size::Max
        ) const -> storage::MaxSize
        ;

    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      [[nodiscard]] auto size_used
        ( typename Base::ReadAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::Size::Used
        ) const -> memory::Size
        ;

    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      [[nodiscard]] auto segment_create
        ( typename Base::WriteAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::Segment::Create
        , memory::Size
        ) -> storage::segment::ID
        ;

    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto segment_remove
        ( typename Base::WriteAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::Segment::Remove
        , storage::segment::ID
        ) -> memory::Size
        ;

    template< storage::is_implementation StorageImplementation
            , chunk::is_access Access
            >
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      [[nodiscard]] auto chunk_description
        ( typename Base::ReadAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::Chunk::Description
        , storage::segment::ID
        , memory::Range
        ) const -> chunk::Description<Access, StorageImplementations...>
        ;

    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto file_read
        ( typename Base::ReadAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::File::Read
        , storage::segment::ID
        , memory::Offset
        , std::filesystem::path
        , memory::Range
        ) const -> memory::Size
        ;

    template<storage::is_implementation StorageImplementation>
      requires ( util::type::List<StorageImplementations...>
                   ::template contains<StorageImplementation>()
               )
      auto file_write
        ( typename Base::ReadAccess const&
        , storage::ID
        , typename StorageImplementation::Parameter::File::Write
        , storage::segment::ID
        , memory::Offset
        , std::filesystem::path
        , memory::Range
        ) const -> memory::Size
        ;
  };
}

#include "detail/Storages.ipp"
