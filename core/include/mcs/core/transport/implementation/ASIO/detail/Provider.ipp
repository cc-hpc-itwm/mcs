// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::core::transport::implementation::ASIO
{
  template< util::ASIO::is_protocol Protocol
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

  template< util::ASIO::is_protocol Protocol
          , storage::is_implementation... StorageImplementations
          >
    auto Provider< Protocol
                 , util::type::List<StorageImplementations...>
                 >::connection_information
      (
      ) const -> util::ASIO::Connectable<Protocol>
  {
    return util::ASIO::make_connectable (_provider.local_endpoint());
  }
}

namespace mcs::core::transport::implementation::ASIO
{
  template< rpc::is_protocol Protocol
          , typename Executor
          , storage::is_implementation... StorageImplementations
          >
    [[nodiscard]] auto make_provider
      ( Executor& executor
      , typename Protocol::endpoint endpoint
      , util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      )
  {
    return Provider< Protocol
                   , util::type::List<StorageImplementations...>
                   >
      {executor, endpoint, storages};
  }
}
