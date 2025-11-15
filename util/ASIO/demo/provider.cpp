// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <cstdlib>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <functional>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/syscall/hostname.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  auto provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        {fmt::format ("usage: {} port {{'v4','v6'}}", args[0])};
    }

    auto endpoint
      { std::invoke
        ( [&]() -> asio::ip::tcp::endpoint
          {
            if (args[2] == std::string {"v4"})
            {
              return asio::ip::tcp::endpoint
                { asio::ip::tcp::v6()
                , mcs::util::read::read<asio::ip::port_type> (args[1])
                };
            }

            if (args[2] == std::string {"v6"})
            {
              return asio::ip::tcp::endpoint
                { asio::ip::tcp::v6()
                , mcs::util::read::read<asio::ip::port_type> (args[1])
                };
            }

            throw std::invalid_argument
              { fmt::format
                  ( "Unknown protocol '{}'. Expected one of {{'v4','v6'}}"
                  , args[2]
                  )
              };
          }
        )
      };
    auto io_context {asio::io_context{}};
    io_context.run();
    auto acceptor {asio::ip::tcp::acceptor {io_context}};
    acceptor.open (endpoint.protocol());
    acceptor.set_option (asio::ip::tcp::acceptor::reuse_address {true});
    acceptor.set_option (asio::ip::tcp::no_delay {true});
    acceptor.bind (endpoint);
    acceptor.listen();

    fmt::print ( "host '{}' port '{}'\n"
               , mcs::util::syscall::hostname()
               , acceptor.local_endpoint().port()
               );

    fmt::print ("waiting for connection\n");

    auto socket {acceptor.accept()};

    fmt::print ("connected\n");

    auto buffer {std::vector<char> (16)};

    fmt::print ("receiving {} bytes...\n", buffer.size());

    auto const bytes_received {asio::read (socket, asio::buffer (buffer))};

    fmt::print ("received {} bytes: {}\n", bytes_received, buffer);

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, provider_main);
}
