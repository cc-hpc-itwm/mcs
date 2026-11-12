// Copyright (C) 2022-2026 Fraunhofer ITWM
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
    auto receive_buffer_with_header (std::shared_ptr<Socket> socket)
      -> asio::awaitable<ReceivedFrame<Header>>
  {
    struct SizeAndHeader
    {
      std::size_t size{};
      Header header{};
    };
    SizeAndHeader prolog;
    co_await asio::async_read
      ( *socket
      , asio::buffer (std::addressof (prolog), sizeof (prolog))
      , asio::use_awaitable
      );
    if (prolog.size < sizeof (Header))
    {
      throw std::logic_error {"receive: Not enough data."};
    }
    auto buffer {detail::Buffer {prolog.size - sizeof (Header)}};
    co_await asio::async_read
      (*socket, buffer.modifiable(), asio::use_awaitable);
    co_return ReceivedFrame<Header>
      {std::move (prolog.header), std::move (buffer)};
  }

}
