// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/use_awaitable.hpp>
#include <type_traits>

namespace mcs::util::ASIO
{
  template<is_protocol Protocol>
    template<typename Executor>
      ListeningAcceptor<Protocol>::ListeningAcceptor
        ( Executor& transport_executor
        , typename Protocol::endpoint endpoint
        )
          : _acceptor {transport_executor}
  {
    _acceptor.open (endpoint.protocol());
    _acceptor.set_option (typename Protocol::acceptor::reuse_address {true});
    if constexpr (std::is_same_v<Protocol, asio::ip::tcp>)
    {
      _acceptor.set_option (asio::ip::tcp::no_delay {true});
    }
    _acceptor.bind (endpoint);
    _acceptor.listen();
  }

  template<is_protocol Protocol>
    auto ListeningAcceptor<Protocol>::local_endpoint
      (
      ) const -> typename Protocol::endpoint
  {
    return _acceptor.local_endpoint();
  }

  template<is_protocol Protocol>
    auto ListeningAcceptor<Protocol>::accept() -> typename Protocol::socket
  {
    return _acceptor.accept();
  }

  template<is_protocol Protocol>
    auto ListeningAcceptor<Protocol>::async_accept
      (
      ) -> asio::awaitable<typename Protocol::socket>
  {
    return _acceptor.async_accept (asio::use_awaitable);
  }
}
