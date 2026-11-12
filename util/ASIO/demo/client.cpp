// Copyright (C) 2023-2026 Fraunhofer ITWM
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
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto host {Parameter {"host"}};
    constexpr auto port {Parameter {"port"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <HOST>\n"
        "    Server hostname or IP address.\n"
        "  --{2} <PORT>\n"
        "    Server port number or resolver service name.\n"
      , args.at (0)
      , option::host
      , option::port
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto client_main (mcs::util::arguments::View args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::host)
        . add (option::port)
        )
      };

    auto io_context {asio::io_context{}};
    auto socket {asio::ip::tcp::socket {io_context}};
    auto const host {options.at (option::host)};
    auto const port {options.at (option::port)};

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
  return mcs::util::main (argc, argv, help, client_main);
}
