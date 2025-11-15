// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/buffer.hpp>
#include <asio/connect.hpp>
#include <asio/read.hpp>
#include <cstdint>
#include <exception>
#include <fmt/format.h>
#include <iterator>
#include <mcs/rpc/detail/Buffer.hpp>
#include <mcs/rpc/detail/make_handshake_data.hpp>
#include <mcs/rpc/error/HandshakeFailed.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/util/ASIO/connected_socket.hpp>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::detail
{
  template< is_protocol Protocol
          , is_command... Commands
          >
    auto hands_shaken
      ( typename Protocol::socket socket
      ) -> typename Protocol::socket
  {
    try
    {
      auto size {std::size_t{}};
      asio::read (socket, asio::buffer (std::addressof (size), sizeof (size)));

      auto buffer {Buffer {size}};
      asio::read (socket, buffer.modifiable());

      using Handshake = decltype (make_handshake_data<Commands...>());

      auto const server_data {buffer.load<Handshake>()};
      auto const client_data {make_handshake_data<Commands...>()};

      auto const cxx23_starts_with
        { [] (auto const& l, auto const& r)
          {
            return std::ranges::mismatch (l, r).in2 == std::cend (r);
          }
        };

      if (!cxx23_starts_with (server_data, client_data))
      {
        throw std::runtime_error
          { fmt::format ( "Not a prefix. Server: {}, Client: {}"
                        , server_data
                        , client_data
                        )
          };
      }
    }
    catch (...)
    {
      std::throw_with_nested (error::HandshakeFailed{});
    }

    return socket;
  }

  template< is_protocol Protocol
          , is_command... Commands
          , typename Executor
          >
    auto connected_socket
      ( Executor& executor
      , typename Protocol::endpoint endpoint
      ) -> std::shared_ptr<typename Protocol::socket>
  {
    return std::make_shared<typename Protocol::socket>
      ( hands_shaken<Protocol, Commands...>
          ( util::ASIO::connected_socket<Protocol>
              ( executor
              , endpoint
              )
          )
      );
  }

  template< is_protocol Protocol
          , is_command... Commands
          , typename Executor
          >
    auto connected_socket
      ( Executor& executor
      , util::ASIO::Connectable<Protocol> connectable
      ) -> std::shared_ptr<typename Protocol::socket>
  {
    return std::make_shared<typename Protocol::socket>
      ( hands_shaken<Protocol, Commands...>
          ( util::ASIO::connected_socket<Protocol>
              ( executor
              , connectable
              )
          )
      );
  }
}
