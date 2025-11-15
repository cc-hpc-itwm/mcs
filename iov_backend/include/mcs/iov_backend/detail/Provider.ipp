// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::iov_backend
{
  template<rpc::is_protocol Protocol, typename ExecutorStoragesClients>
    template<typename Executor>
      Provider<Protocol, ExecutorStoragesClients>::Provider
        ( typename Protocol::endpoint endpoint
        , Executor& executor
        , ExecutorStoragesClients& io_context_storages_clients
        , util::not_null<provider::StoragesClients> storages_clients
        , provider::State state
        )
          : _state {state}
          , _provider { endpoint
                      , executor
                      , io_context_storages_clients
                      , storages_clients
                      , _state
                      }
  {}

  template<rpc::is_protocol Protocol, typename ExecutorStoragesClients>
    auto Provider<Protocol, ExecutorStoragesClients>::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _provider.local_endpoint();
  }
}
