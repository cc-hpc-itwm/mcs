// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/ASIO/Connectable.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::block_device
{
  template<typename MakeTransportClient, typename MetaDataClient>
      Reader<MakeTransportClient, MetaDataClient>::Reader
        ( MetaDataClient& meta_data_client
        , MakeTransportClient&& make_transport_client
        )
          : _meta_data_client {meta_data_client}
          , _make_transport_client
              {std::forward<MakeTransportClient> (make_transport_client)}
  {}
  template<typename MakeTransportClient, typename MetaDataClient>
    template<typename... Args>
    requires (std::is_constructible_v<MakeTransportClient, Args...>)
      Reader<MakeTransportClient, MetaDataClient>::Reader
        ( Create
        , MetaDataClient& meta_data_client
        , Args&&... args
        )
          : _meta_data_client {meta_data_client}
          , _make_transport_client {std::forward<Args> (args)...}
  {}

  template<typename MakeTransportClient, typename MetaDataClient>
    template<core::storage::is_implementation... StorageImplementations>
      auto Reader<MakeTransportClient, MetaDataClient>::operator()
        ( block::ID block_id
        , core::transport::Address const& destination
        ) -> std::future<core::memory::Size>
  {
    auto const location {_meta_data_client.location (block_id)};

    auto const block_size
      {core::memory::make_size (_meta_data_client.block_size())};

    return util::ASIO::run
      ( location.provider
      , [&]<util::ASIO::is_protocol CommandProtocol>
          (util::ASIO::Connectable<CommandProtocol> provider)
        {
          return _make_transport_client (provider)
            . memory_get
              ( destination
              , location.address
              , block_size
              )
              ;
        }
      );
  }
}

namespace mcs::block_device
{
  template< typename MakeTransportClient
          , typename MetaDataClient
          , typename... Args
          >
    requires (std::is_constructible_v<MakeTransportClient, Args...>)
    auto make_reader
      ( MetaDataClient& meta_data_client
      , Args&&... args
      )
  {
    return Reader<MakeTransportClient, MetaDataClient>
      { typename Reader<MakeTransportClient, MetaDataClient>::Create{}
      , meta_data_client
      , std::forward<Args> (args)...
      };
  }
}
