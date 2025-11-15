// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/connect.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>
#include <cstdlib>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <mcs/util/main.hpp>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace
{
  auto client_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        {fmt::format ("usage: {} host port", args[0])};
    }

    auto io_context {asio::io_context{}};
    auto socket {asio::ip::tcp::socket {io_context}};
    auto const host {args[1]};
    auto const port {args[2]};

    fmt::print ("connecting to {}, {}...\n", host, port);

    asio::connect
      ( socket
      , asio::ip::tcp::resolver {io_context}.resolve (host, port)
      );

    fmt::print ("connected to {}, {}\n", host, port);

    auto buffer {std::vector<char> (16)};
    std::iota (std::begin (buffer), std::end (buffer), 'a');

    fmt::print ("write {} bytes: {}...\n", buffer.size(), buffer);
    auto const bytes_written (asio::write (socket, asio::buffer (buffer)));

    fmt::print ("written {} bytes\n", bytes_written);

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, client_main);
}
