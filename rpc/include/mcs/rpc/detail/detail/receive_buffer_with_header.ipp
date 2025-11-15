// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <asio/use_awaitable.hpp>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mcs::rpc::detail
{
  template<typename Header, typename Socket>
    auto receive_buffer_with_header (Socket& socket)
      -> asio::awaitable<std::tuple<Header, Buffer>>
  {
    struct SizeAndHeader
    {
      std::size_t size{};
      Header header{};
    };
    SizeAndHeader prolog;
    co_await asio::async_read
      ( socket
      , asio::buffer (std::addressof (prolog), sizeof (prolog))
      , asio::use_awaitable
      );
    if (prolog.size < sizeof (Header))
    {
      throw std::logic_error {"receive: Not enough data."};
    }
    auto buffer {detail::Buffer {prolog.size - sizeof (Header)}};
    co_await asio::async_read
      (socket, buffer.modifiable(), asio::use_awaitable);
    co_return std::make_tuple
      (std::move (prolog.header), std::move (buffer));
  }

  template<typename Protocol, typename Header, typename ReadLock>
    auto receive_buffer_with_header
      ( std::shared_ptr<typename Protocol::socket> socket
      , ReadLock
      ) -> asio::awaitable<std::tuple<Header, Buffer>>
  {
    auto response {co_await receive_buffer_with_header<Header> (*socket)};

    co_return response;
  }

  template<typename Protocol, typename Header>
    auto receive_buffer_with_header
      ( std::shared_ptr<typename Protocol::socket> socket
      ) -> asio::awaitable<std::tuple<Header, Buffer>>
  {
    auto response {co_await receive_buffer_with_header<Header> (*socket)};

    co_return response;
  }
}
