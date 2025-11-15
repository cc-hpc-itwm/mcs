// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::block_device
{
  template<typename MakeTransportClient, typename MetaDataClient>
    Writer<MakeTransportClient, MetaDataClient>::Writer
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
    Writer<MakeTransportClient, MetaDataClient>::Writer
        ( Create
        , MetaDataClient& meta_data_client
        , Args&&... args
        )
          : _meta_data_client {meta_data_client}
          , _make_transport_client {std::forward<Args> (args)...}
  {}

  template<typename MakeTransportClient, typename MetaDataClient>
    template<core::storage::is_implementation... StorageImplementations>
      auto Writer<MakeTransportClient, MetaDataClient>::operator()
        ( block::ID block_id
        , core::transport::Address const& source
        ) -> std::future<core::memory::Size>
  {
    auto const location {_meta_data_client.location (block_id)};

    auto const block_size
      {core::memory::make_size (_meta_data_client.block_size())};

    return util::ASIO::run
      ( location.provider
      , [&]<util::ASIO::is_protocol Protocol>
          (util::ASIO::Connectable<Protocol> provider)
        {
          return _make_transport_client (provider)
            . memory_put
              ( location.address
              , source
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
    auto make_writer
      ( MetaDataClient& meta_data_client
      , Args&&... args
      )
  {
    return Writer<MakeTransportClient, MetaDataClient>
      { typename Writer<MakeTransportClient, MetaDataClient>::Create{}
      , meta_data_client
      , std::forward<Args> (args)...
      };
  }
}
