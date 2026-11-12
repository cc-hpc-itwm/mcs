// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <cstdlib>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <functional>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/hostname.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto ip_version {Parameter {"ip-version"}};
    constexpr auto port {Parameter {"port"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <PORT>\n"
        "    TCP listening port number.\n"
        "  --{2} <'v4'|'v6'>\n"
        "    IP protocol version.\n"
      , args.at (0)
      , option::port
      , option::ip_version
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  auto provider_main (mcs::util::arguments::View args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::port)
        . add (option::ip_version)
        )
      };
    auto const port_text {options.at (option::port)};

    auto endpoint
      { std::invoke
        ( [&]() -> asio::ip::tcp::endpoint
          {
            if (options.at (option::ip_version) == std::string {"v4"})
            {
              return asio::ip::tcp::endpoint
                { asio::ip::tcp::v4()
                , mcs::util::read::read<asio::ip::port_type> (port_text)
                };
            }

            if (options.at (option::ip_version) == std::string {"v6"})
            {
              return asio::ip::tcp::endpoint
                { asio::ip::tcp::v6()
                , mcs::util::read::read<asio::ip::port_type> (port_text)
                };
            }

            throw std::invalid_argument
              { fmt::format
                  ( "Unknown protocol '{}'. Expected one of {{'v4','v6'}}"
                  , options.at (option::ip_version)
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
  return mcs::util::main (argc, argv, help, provider_main);
}
