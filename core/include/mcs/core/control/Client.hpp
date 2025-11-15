// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/chunk/Description.hpp>
#include <mcs/core/control/Commands.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/type/List.hpp>
#include <type_traits>

namespace mcs::core::control
{
  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , typename StorageImplementations
          > struct Client;

  template< util::ASIO::is_protocol Protocol
          , rpc::is_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    struct Client< Protocol
                 , AccessPolicy
                 , util::type::List<StorageImplementations...>
                 >
    : public provider::Commands<StorageImplementations...>
               ::template wrap<rpc::Client, Protocol, AccessPolicy>
  {
    using Base
      = provider::Commands<StorageImplementations...>
          ::template wrap<rpc::Client, Protocol, AccessPolicy>
      ;

    template<typename Executor>
      Client ( Executor&
             , util::ASIO::Connectable<Protocol>
             );

    // --------------------------------------------------------------------
    // Storages:

    // Returns: The ID of the new storage.
    //
    // Ensures: The storage has been added to the known storages.
    //
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::Create
    //
    // Note: No attempt is taken to prevent concurrent access to a
    // single storage. It is up to users to either ensure sequential
    // access or to use storages that are ready to handle concurrent
    // accesses.
    //
    // EXAMPLE:
    //
    //     using SHMEM = mcs::core::storage::implementation::SHMEM;
    //     auto const shmem_id
    //       { client.template storage_create<SHMEM>
    //           ("unique_shmem_prefix", max_size)
    //       };
    //
    template<storage::is_implementation StorageImplementation>
      [[nodiscard]] auto storage_create
        ( typename StorageImplementation::Parameter::Create
        ) const -> storage::ID
      ;

    // Ensures: The storage has been removed from the known storages.
    //
    // If called multiple times with the same id, then the first call
    // will remove the storage and later calls will do nothing.
    //
    // Note: Not attempt is taken to prevent from removing storages
    // that are in use. it us up to users to make sure the storage is
    // not and will not be in use.
    //
    auto storage_remove
      ( storage::ID
      ) const -> void
      ;

    // Returns: The maximum size of the storage with the given ID.
    //
    // Expects: The storage exists.
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::Size::Max
    //
    // EXAMPLE:
    //
    //     auto const max_size
    //       { client.storage_size_max
    //         ( storage_id
    //         , storage::make_parameter (Files::Parameter::Size::Max{})
    //         )
    //       };
    //
    [[nodiscard]] auto storage_size_max
      ( storage::ID
      , storage::Parameter
      ) const -> storage::MaxSize
      ;

    // Returns: The currently used size of the storage with the given
    //          ID. (Note: Race!)
    //
    // Expects: The storage exists.
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::Size::Used
    //
    // EXAMPLE:
    //
    //     auto const used
    //       { client.storage_size_used
    //         ( storage_id
    //         , storage::make_parameter (SHMEM::Parameter::Size::Used{})
    //         )
    //       };
    //
    [[nodiscard]] auto storage_size_used
      ( storage::ID
      , storage::Parameter
      ) const -> memory::Size
      ;

    // Returns: The maximum size and the currently used size of the
    // storage with the given ID. The command is a combination of
    // size_max() and size_used() into a single command.
    //
    // Expects: The storage exists.
    // Expects: The parameters can be deserialized into
    //          Storage::Parameter::Size::Max and
    //          Storage::Parameter::Size::Used.
    //
    // EXAMPLE:
    //
    //     auto const size
    //       { client.storage_size
    //         ( storage_id
    //         , storage::make_parameter (Files::Parameter::Size::Max{})
    //         , storage::make_parameter (Files::Parameter::Size::Used{})
    //         )
    //       };
    //     auto const available {size.max() - size.used()};
    //
    [[nodiscard]] auto storage_size
      ( storage::ID
      , storage::Parameter parameter_size_max
      , storage::Parameter parameter_size_used
      ) const -> command::storage::Size::Response
      ;

    // --------------------------------------------------------------------
    // Segments:

    // Returns: The ID of the new segment.
    //
    // Ensures: A memory segment on the given storage has been created.
    //
    // Expects: The storage exists.
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::Segment::Create
    //
    // EXAMPLE:
    //
    //    auto const segment_id
    //      { client.segment_create
    //          ( shmem_id
    //          , storage::make_parameter (SHMEM::Parameter::Segment::Create{})
    //          , memory::make_size (20 << 20)
    //          )
    //      };
    //
    [[nodiscard]] auto segment_create
      ( storage::ID
      , storage::Parameter
      , memory::Size
      ) const -> storage::segment::ID
      ;

    // Returns: The amount of memory freed.
    //
    // Ensures: The segment has been erased from the given storage.
    //
    // Expects: The storage exists.
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::Segment::Remove
    //
    // EXAMPLE:
    //
    //     client.segment_remove
    //       ( shmem_id
    //       , storage::make_parameter (SHMEM::Parameter::Segment::Remove{})
    //       , segment_id
    //       )
    //
    auto segment_remove
      ( storage::ID
      , storage::Parameter
      , storage::segment::ID
      ) const -> memory::Size
      ;

    // --------------------------------------------------------------------
    // Direct access:

    // Returns: A chunk description that allows for creation of a
    // chunk with direct access.
    //
    // Expects: The storage exists.
    // Expects: The segment exists.
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::Chunk::Description
    // Expects: The range is a subrange of the segment.
    //
    // Note: Chunks allow for direct access and no copies are
    // involved. Fails if direct access without copy is impossible.
    //
    // EXAMPLE:
    //
    //     auto chunk
    //       { mcs::core::Chunk< mcs::core::chunk::access::Mutable
    //                         , StorageImplementations...
    //                         >
    //         { client.chunk_description<mcs::core::chunk::access::Mutable>
    //           ( storage_id
    //           , storage::make_parameter
    //               (Files::Parameter::Chunk::Description{})
    //           , segment_id
    //           , memory_make_range (memory::make_offset (0), number_of_bytes)
    //           )
    //         }
    //       };
    //     std::ranges::fill (as<int> (chunks), 42);
    //
    template<chunk::is_access Access>
      [[nodiscard]] auto chunk_description
        ( storage::ID
        , storage::Parameter
        , storage::segment::ID
        , memory::Range
        ) const -> chunk::Description<Access, StorageImplementations...>
      ;

    // --------------------------------------------------------------------
    // File I/O:

    // Tell the provider to execute file I/O. The provider will call
    // file_{read, write} provided by the storage implementation.
    //
    // Expects: The parameter can be deserialized into
    //          Storage::Parameter::File::{Read, Write}
    //
    // Note: File I/O is _unsychronized_ in the sense that it is
    // possible to remove segments or storages that are used in the
    // file I/O. Doing so results in undefined behavior. All
    // parameters must be valid until the file I/O operations have
    // been completed.
    //
    // Returns: The number of bytes read/written.

    // Read data from a file into a storage. The data in the given
    // range in the file is read and copied into the given segment,
    // starting at the given offset.
    //
    // Pre: The segment has, starting at the given offset, space for
    // size (range) many bytes.
    //
    auto file_read
      ( storage::ID
      , storage::Parameter
      , storage::segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;

    // Write data from a storage into a file. The data in the given
    // segment and starting at the given offset is copied into the
    // given range in the given file.
    //
    // Pre: The segment does, starting at the given offset, contain at
    // least size (range) many bytes.
    // Post: The file size is at least begin (range) + size (range).
    //
    auto file_write
      ( storage::ID
      , storage::Parameter
      , storage::segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;
  };
}

#include "detail/Client.ipp"
