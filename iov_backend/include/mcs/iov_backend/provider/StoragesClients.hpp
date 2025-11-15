// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/chunk/Description.hpp>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/iov_backend/provider/state/Collections.hpp>
#include <mcs/iov_backend/provider/state/Storages.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace mcs::iov_backend::provider
{
  struct State;

  template<typename S>
    concept is_capacity_sink =
      std::is_invocable_v<S, storage::ID, core::storage::MaxSize>
    ;

  struct StoragesClients
  {
  private:
    friend struct State;

    // Adds a storage.
    //
    // Requires: The storage_id has not been used before.
    // Ensures: There is a connection to the storages provider.
    //
    template<typename Executor>
      [[nodiscard]] auto add
        ( Executor&
        , storage::ID
        , Storage const&
        ) -> storage::ID
      ;

    // Retrieve the capacities of the storages. For each retrieved
    // capacity the sink is invoked. The capacities are received with
    // no specific order, exactly once for each storage.
    //
    template<is_capacity_sink CapacitySink>
      auto capacities
        ( state::Storages const&
        , CapacitySink
        ) -> void
      ;

    // Restarts with the given storages an collections.
    //
    // Ensures: There is a connection to each of the storages.
    // Ensures: The chunk_description of all collections can be retrieved.
    //
    template<typename Executor>
      auto restart
        ( Executor&
        , state::Storages const&
        , state::Collections const&
        ) -> void
      ;

    // Creates a new collection.
    //
    // Requires: All storages have been added before.
    // Requires: No range exceeds the maximum size of the
    // corresponding storage.
    // Ensures: Segments are created on all storages.
    //
    template<typename MakeRange>
      requires
      ( std::regular_invocable<MakeRange, storage::ID, Storage const&>
        &&
        std::is_same_v
          < std::invoke_result_t<MakeRange, storage::ID, Storage const&>
          , core::memory::Range
          >
      )
      [[nodiscard]] auto create
        ( state::Storages const&
        , MakeRange
        ) -> UsedStorages
      ;

    // Removes the segments mentioned in the used_storages.
    //
    auto remove (state::Storages const&, UsedStorages const&) -> void;

    // Retrieves the chunk_descriptions for all chunks mentioned in
    // the all of the collections.
    //
    template<core::chunk::is_access Access>
      auto chunk_description
        ( state::Storages const&
        , state::Collections const&
        ) -> void;

    using AccessPolicy = rpc::access_policy::Sequential;

    template<rpc::is_protocol Protocol>
      using Client = core::control::Client
        < Protocol
        , AccessPolicy
        , SupportedStorageImplementations
        >
      ;
    // \todo use HeterogeneousMap
    using AnyClient = std::variant
      < Client<asio::ip::tcp>
      , Client<asio::local::stream_protocol>
      >;

    // \todo max number of connections and eviction policy
    std::unordered_map<storage::ID, AnyClient> _clients_by_storage_id;

    [[nodiscard]] auto at (storage::ID) -> AnyClient&;
  };
}

#include "detail/StoragesClients.ipp"
