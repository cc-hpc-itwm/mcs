// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Chunk.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/Commands.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/ASIO/Connectable.hpp>

namespace mcs::share_service
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    struct Client : public Commands::template wrap< rpc::Client
                                                  , Protocol
                                                  , AccessPolicy
                                                  >

  {
    using Base = Commands::template wrap< rpc::Client
                                        , Protocol
                                        , AccessPolicy
                                        >;

    // Create a client of the connectable share_service.
    //
    template<typename Executor>
      Client (Executor&, util::ASIO::Connectable<Protocol>);

    // Create a chunk of memory.
    //
    // Ensures: The chunk exists. Throws if the requested size exceeds
    // the max_size given in the parameters.
    //
    // Returns: The chunk.
    //
    template<is_supported_storage_implementation StorageImplementation>
      auto create
        ( core::memory::Size
        , command::create::Parameters<StorageImplementation>
        ) const -> Chunk
      ;

    // Attach to a chunk. Multiple clients might attach to the same
    // chunk and no attempt is made to synchronize their accesses.
    //
    // Expects: The chunk exists.
    //
    // Returns: A core-chunk that provides direct access to the memory.
    //
    template< core::chunk::is_access Access
            , is_supported_storage_implementation StorageImplementation
            >
      auto attach
        ( Chunk
        , command::attach::Parameters<StorageImplementation>
        ) const -> SupportedStorageImplementations::template wrap< core::Chunk
                                                                 , Access
                                                                 >
      ;

    // Remove a chunk.
    //
    // Expects: The chunk exists.
    //
    template<is_supported_storage_implementation StorageImplementation>
      auto remove
        ( Chunk
        , command::remove::Parameters<StorageImplementation>
        ) const -> void
      ;

  private:
    util::ASIO::Connectable<Protocol> _provider_connectable;
  };
}

#include "detail/Client.ipp"
