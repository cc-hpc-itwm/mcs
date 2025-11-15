// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/command/Locations.hpp>
#include <mcs/iov_backend/command/Range.hpp>
#include <mcs/iov_backend/command/State.hpp>
#include <mcs/iov_backend/command/collection/Create.hpp>
#include <mcs/iov_backend/command/collection/Delete.hpp>
#include <mcs/iov_backend/command/storage/Add.hpp>
#include <mcs/iov_backend/provider/State.hpp>
#include <mcs/iov_backend/provider/StoragesClients.hpp>
#include <mcs/util/not_null.hpp>
#include <shared_mutex>

namespace mcs::iov_backend::provider
{
  using StorageID = std::size_t;

  template<typename ExecutorStoragesClients>
    struct Handler
  {
    Handler
      ( ExecutorStoragesClients&
      , util::not_null<StoragesClients>
      , State&
      );

    auto operator() (command::State) const -> command::State::Response;
    auto operator() (command::Locations) const -> command::Locations::Response;
    auto operator() (command::Range) const -> command::Range::Response;
    auto operator()
      ( command::collection::Append
      ) -> command::collection::Append::Response
      ;
    auto operator()
      ( command::collection::Create
      ) -> command::collection::Create::Response
      ;
    auto operator()
      ( command::collection::Delete
      ) -> command::collection::Delete::Response
      ;
    auto operator() (command::storage::Add) -> command::storage::Add::Response;

  private:
    ExecutorStoragesClients& _io_context_storages_clients;
    mutable std::shared_mutex _guard;
    util::not_null<StoragesClients> _storages_clients;
    State& _state;
  };
}

#include "detail/Handler.ipp"
