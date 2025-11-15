// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/iov_backend/provider/Handler.hpp>
#include <mutex>

namespace mcs::iov_backend::provider
{
  template<typename ExecutorStoragesClients>
    Handler<ExecutorStoragesClients>::Handler
      ( ExecutorStoragesClients& io_context_storages_client
      , util::not_null<StoragesClients> storages_clients
      , State& state
      )
        : _io_context_storages_clients {io_context_storages_client}
        , _storages_clients {storages_clients}
        , _state {state}
  {}

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::storage::Add add
      ) -> command::storage::Add::Response
  {
    auto const lock {std::unique_lock {_guard}};

    return { _state.add ( _io_context_storages_clients
                        , _storages_clients
                        , add.storage
                        )
           };
  }

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::collection::Append collection_append
      ) -> command::collection::Append::Response
  {
    auto const lock {std::unique_lock {_guard}};

    return _state.collection_append
      ( _storages_clients
      , collection_append._collection_id
      , collection_append._range_to_append
      );
  }

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::collection::Create collection_create
      ) -> command::collection::Create::Response
  {
    auto const lock {std::unique_lock {_guard}};

    return _state.collection_create
      ( _storages_clients
      , collection_create._collection_id
      , collection_create._size
      );
  }

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::collection::Delete collection_delete
      ) -> command::collection::Delete::Response
  {
    auto const lock {std::unique_lock {_guard}};

    return _state.collection_delete
      ( _storages_clients
      , collection_delete._collection_id
      );
  }

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::State
      ) const -> command::State::Response
  {
    auto const lock {std::shared_lock {_guard}};

    return _state;
  }

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::Locations command_locations
      ) const -> command::Locations::Response
  {
    auto const lock {std::shared_lock {_guard}};

    return _state.locations
      ( command_locations._collection_id
      , command_locations._range
      );
  }

  template<typename ExecutorStoragesClients>
    auto Handler<ExecutorStoragesClients>::operator()
      ( command::Range command_range
      ) const -> command::Range::Response
  {
    auto const lock {std::shared_lock {_guard}};

    return _state.range (command_range._collection_id);
  }
}
