// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdint>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <tuple>

TEST (util_ASIO_ip_tcp_endpoint, read_default_constructed_endpoint)
{
  auto const input {fmt::format ("ip::tcp()")};
  ASSERT_EQ ( asio::ip::tcp::endpoint{}
            , mcs::util::read::read<asio::ip::tcp::endpoint> (input)
            );
}

struct util_ASIO_ip_tcp_endpointR : public mcs::testing::random::Test{};

TEST_F (util_ASIO_ip_tcp_endpointR, read_endpoint_with_address)
{
  auto const port {mcs::testing::random::value<asio::ip::port_type>{}()};
  auto net {mcs::testing::random::value<std::uint8_t>{}};
  auto const [a, b, c, d] {std::make_tuple (net(), net(), net(), net())};
  auto const input
    {fmt::format ( "ip::tcp (\"{}.{}.{}.{}\", {})"
                 , a
                 , b
                 , c
                 , d
                 , port
                 )
    };
  ASSERT_EQ ( ( asio::ip::tcp::endpoint
                { asio::ip::make_address
                    (fmt::format ("{}.{}.{}.{}", a, b, c, d))
                , port
                }
              )
            , mcs::util::read::read<asio::ip::tcp::endpoint> (input)
            );
}

TEST (util_ASIO_local_stream_protocol_endpoint, read_with_empty_path)
{
  auto const input {fmt::format ("local::stream_protocol(\"\")")};
  ASSERT_EQ ( asio::local::stream_protocol::endpoint {""}
            , mcs::util::read::read<asio::local::stream_protocol::endpoint>
                (input)
            );
}
