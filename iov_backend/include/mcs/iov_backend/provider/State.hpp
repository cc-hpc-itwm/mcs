// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/core/memory/Range.hpp>
#include <mcs/iov_backend/Locations.hpp>
#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/iov_backend/provider/StoragesClients.hpp>
#include <mcs/iov_backend/provider/state/Collections.hpp>
#include <mcs/iov_backend/provider/state/Storages.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/not_null.hpp>

namespace mcs::iov_backend::provider
{
  template<typename> struct Handler;
}

namespace mcs::iov_backend::provider
{
  struct State
  {
    State() = default;

    // Creates a new collection.
    // Pre: The collection does not exist.
    // Post: The collection exists.
    // Returns: The used storages.
    //
    auto collection_create
      ( util::not_null<StoragesClients>
      , collection::ID
      , core::memory::Size
      ) -> UsedStorages
      ;
    // Appends a range to a collection.
    // Pre: The collection does exist.
    // Post: The collection is grown such that the given range fits
    // into the collection. If the collection is large enough, then no
    // additional space is allocated.
    // Returns: The size of the collection.
    //
    auto collection_append
      ( util::not_null<StoragesClients>
      , collection::ID
      , core::memory::Range
      ) -> core::memory::Size
      ;

    auto collection_delete
      ( util::not_null<StoragesClients>
      , collection::ID
      ) -> void
      ;

    [[nodiscard]] auto range
      ( collection::ID
      ) const -> core::memory::Range
      ;

    [[nodiscard]] auto locations
      ( collection::ID
      , core::memory::Range
      ) const -> Locations
      ;

    template<typename Executor>
      auto restart (Executor&, util::not_null<StoragesClients>) -> State&;

    [[nodiscard]] auto collections() const -> state::Collections
    {
      return _collections;
    }
    [[nodiscard]] auto storages() const -> state::Storages
    {
      return _storages;
    }


  private:
    // \note Synchronization is done by provider::Handler.
    template<typename> friend struct provider::Handler;

    storage::ID _next_storage_id{};
    template<typename Executor>
      auto add ( Executor&
               , util::not_null<StoragesClients>
               , Storage
               ) -> storage::ID
      ;

    state::Storages _storages;
    state::Collections _collections;

    MCS_SERIALIZATION_ACCESS();
    State (storage::ID, state::Storages, state::Collections);

    auto at
      ( collection::ID
      ) const -> decltype (_collections)::const_iterator
      ;
    auto at
      ( collection::ID
      ) -> decltype (_collections)::iterator
      ;

    auto distribute_as_equal_as_possible
      ( util::not_null<StoragesClients>
      , core::memory::Size
      , core::memory::Offset
      ) const -> UsedStorages
      ;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( iov_backend::provider::State
      );
}

#include "detail/State.ipp"
