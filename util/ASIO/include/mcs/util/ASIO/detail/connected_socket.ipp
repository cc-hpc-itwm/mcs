// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/connect.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <fmt/format.h>
#include <list>
#include <mcs/util/ASIO/SetSocketOptions.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/overloaded.hpp>
#include <type_traits>
#include <utility>

namespace mcs::util::ASIO
{
  template<is_protocol Protocol, typename Executor>
    auto connected_socket
      ( Executor& executor
      , typename Protocol::endpoint endpoint
      ) -> typename Protocol::socket
  {
    auto socket {typename Protocol::socket {executor}};
    asio::connect (socket, std::list {endpoint});
    SetSocketOptions<Protocol>{} (socket);
    return socket;
  }

  template<is_protocol Protocol, typename Executor>
    auto connected_socket
      ( Executor& executor
      , Connectable<Protocol> connectable
      ) -> typename Protocol::socket
  {
    if constexpr (std::is_same_v<Protocol, asio::local::stream_protocol>)
    {
      return connected_socket<Protocol>
        ( executor
        , typename Protocol::endpoint {connectable.path}
        );
    }
    else
    {
      static_assert (std::is_same_v<Protocol, asio::ip::tcp>);

      return std::visit
        ( util::overloaded
          ( [&] (Connectable<asio::ip::tcp>::Address address)
            {
              return connected_socket<Protocol>
                ( executor
                , typename Protocol::endpoint
                    { asio::ip::make_address (address.address_string)
                    , connectable.port
                    }
                );
            }
          , [&] (Connectable<asio::ip::tcp>::Hostname hostname)
            {
              auto socket {typename Protocol::socket {executor}};

              auto port_string {fmt::format ("{}", connectable.port)};
              auto host_string {std::string {hostname.hostname}};

              using Resolver = typename Protocol::resolver;
              asio::connect
                ( socket
                , Resolver {executor}.resolve
                  ( host_string
                  , port_string
                  , Resolver::address_configured
                  | Resolver::all_matching
                  | Resolver::v4_mapped
                  | Resolver::numeric_service
                  )
                );

              SetSocketOptions<Protocol>{} (socket);

              return socket;
            }
          )
        , connectable.address_or_hostname
        );
    }
  }
}
