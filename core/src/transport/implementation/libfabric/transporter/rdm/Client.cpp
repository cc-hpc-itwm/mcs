// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Client.hpp>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  Client::Client
    ( libfabric::Interface interface
    , libfabric::Name name
    )
      : Interface (interface)
      , _provider
          { std::invoke
            ( [&]
              {
                auto fi_addr {fi_addr_t {FI_ADDR_UNSPEC}};

                _av->insert_single_address
                  ( name.value().data()
                  , std::addressof (fi_addr)
                  , 0
                  , nullptr
                  );

                return fi_addr;
              }
            )
          }
  {}

  auto Client::send (std::span<std::byte const> data) -> void
  {
    return Interface::send (data, _provider);
  }
}
