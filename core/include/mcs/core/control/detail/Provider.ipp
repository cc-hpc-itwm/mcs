// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::core::control
{
  template< rpc::is_protocol Protocol
          , storage::is_implementation... StorageImplementations
          >
    template<typename Executor>
      Provider< Protocol
              , util::type::List<StorageImplementations...>
              >::Provider
        ( Executor& executor
        , typename Protocol::endpoint endpoint
        , util::not_null<Storages<util::type::List<StorageImplementations...>>>
            storages
        )
          : _provider
            { rpc::make_provider<Protocol, Dispatcher>
              ( endpoint
              , executor
              , storages
              )
            }
  {}

  template< rpc::is_protocol Protocol
          , storage::is_implementation... StorageImplementations
          >
    auto Provider< Protocol
                 , util::type::List<StorageImplementations...>
                 >::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _provider.local_endpoint();
  }
}
