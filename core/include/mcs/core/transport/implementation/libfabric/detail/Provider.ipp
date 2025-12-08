// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/ASIO/Connectable.hpp>

namespace mcs::core::transport::implementation::libfabric
{
  template< util::ASIO::is_protocol Protocol
          , transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    template<typename Executor>
      Provider< Protocol
              , Transporter
              , util::type::List<StorageImplementations...>
              >::Provider
        ( Executor& executor
        , typename Protocol::endpoint endpoint
        , libfabric::Interface transporter_interface
        , util::not_null< Storages<util::type::List<StorageImplementations...>>
                        > storages
        )
          : _transporter {transporter_interface}
          , _controller
            { mcs::rpc::make_provider<Protocol, Dispatcher>
              ( endpoint
              , executor
              , util::not_null<Transporter> (std::addressof (_transporter))
              , storages
              )
            }
  {}

  template< util::ASIO::is_protocol Protocol
          , transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    auto Provider< Protocol
                 , Transporter
                 , util::type::List<StorageImplementations...>
                 >::connection_information
      (
      ) const -> provider::ConnectionInformation<Protocol>
  {
    return provider::ConnectionInformation
      { util::ASIO::make_connectable (_controller.local_endpoint())
      , _transporter.name()
      };
  }
}
