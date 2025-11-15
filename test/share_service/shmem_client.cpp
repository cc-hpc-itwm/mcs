// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <functional>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <latch>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/share_service/Provider.hpp>
#include <mcs/share_service/shmem_client.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/UniqID.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/cast.hpp>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace mcs::share_service
{
  namespace
  {
    template<rpc::is_protocol Protocol>
      struct RunningProvider
    {
      template<typename... Args>
      RunningProvider (Args&&... args)
        : _protocol_state {std::forward<Args> (args)...}
      {
        _started.wait();
      }
      ~RunningProvider()
      {
        _io_context.stop();
        _provider.wait();
      }
      [[nodiscard]] auto local_endpoint
        (
        ) const -> typename Protocol::endpoint const&
      {
        return _local_endpoint;
      }

      RunningProvider (RunningProvider const&) = delete;
      RunningProvider (RunningProvider&&) = delete;
      auto operator= (RunningProvider const&) -> RunningProvider& = delete;
      auto operator= (RunningProvider&&) -> RunningProvider& = delete;

    private:
      rpc::ScopedRunningIOContext _io_context
        {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM};
      testing::RPC::ProtocolState<Protocol> _protocol_state;
      std::latch _started {1};
      typename Protocol::endpoint _local_endpoint;
      std::future<void> _provider
        { std::async
          ( std::launch::async
          , [&]
            {
              auto const provider
                { share_service::Provider<Protocol>
                  { _io_context
                  , _protocol_state.local_endpoint()
                  }
                };

              _local_endpoint = provider.local_endpoint();

              _started.count_down();

              _io_context.join();
            }
          )
        };
    };

    template<rpc::is_protocol Protocol> struct ClientState;
    template<> struct ClientState<asio::local::stream_protocol>
    {
      ClientState
        ( RunningProvider<asio::local::stream_protocol> const& running_provider
        )
          : _endpoint_path {running_provider.local_endpoint().path()}
      {}

      std::string _endpoint_path;
      char const* socket {_endpoint_path.c_str()};
    };
    template<> struct ClientState<asio::ip::tcp>
    {
      ClientState
        ( RunningProvider<asio::ip::tcp> const& running_provider
        )
          : port {running_provider.local_endpoint().port()}
      {}

      unsigned short int port;
    };

    struct LocalStream
    {
      using Protocol = asio::local::stream_protocol;
      auto tag() { return "local_stream"; }
    };
    struct TCPv4
    {
      using Protocol = asio::ip::tcp;
      [[nodiscard]] auto tag() const { return "tcpv4"; }
      using ClientCtorArg = TCPVersion::V4;
      [[nodiscard]] auto tcp_version() const { return asio::ip::tcp::v4(); }
    };
    struct TCPv6
    {
      using Protocol = asio::ip::tcp;
      [[nodiscard]] auto tag() { return "tcpv6"; }
      using ClientCtorArg = TCPVersion::V6;
      [[nodiscard]] auto tcp_version() const { return asio::ip::tcp::v6(); }
    };

    using Protocols = ::testing::Types
      < LocalStream
      , TCPv4
      , TCPv6
      >;

    template<class> struct MCSShareServiceShmemSocketClientT
      : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSShareServiceShmemSocketClientT, Protocols);
  }


  TYPED_TEST (MCSShareServiceShmemSocketClientT, create_write_cat_remove)
  {
    using Protocol = typename TypeParam::Protocol;

    auto const running_provider
      { std::invoke
        ( [&]
          {
            if constexpr (std::is_same_v<Protocol, asio::local::stream_protocol>)
            {
              return RunningProvider<Protocol>
                { TypeParam{}.tag()
                };
            }
            else
            {
              return RunningProvider<Protocol>
                { TypeParam{}.tcp_version()
                , TypeParam{}.tag()
                };
            }
          }
        )
      };

    auto const client_state {ClientState<Protocol> (running_provider)};
    auto const client
      { std::invoke
        ( [&]() noexcept
          {
            if constexpr (std::is_same_v<Protocol, asio::local::stream_protocol>)
            {
              return make_shmem_local_stream_client
                ( shmem_client::NumberOfThreads {1u}
                , client_state.socket
                , SIGINT
                , SIGTERM
                );
            }
            else
            {
              return make_shmem_tcp_client
                ( shmem_client::NumberOfThreads {1u}
                , typename TypeParam::ClientCtorArg{}
                , client_state.port
                , SIGINT
                , SIGTERM
                );
            }
          }
        )
      };
    ASSERT_TRUE (client.ok());

    // note: usage of the client does no depend in any way from the
    // protocol

    using RandomSize = testing::random::value<std::uint64_t>;
    auto const size {RandomSize {RandomSize::Max {1 << 20}}()};

    auto const prefix {testing::UniqID {"shmem_client_test"}()};
    auto const chunk
      {client.create (shmem_client::Size {size}, prefix.c_str())};
    ASSERT_TRUE (chunk.ok());

    using Element = int;
    auto const number_of_elements {size / sizeof (Element)};
    auto elements {std::vector<Element> (number_of_elements)};
    std::ranges::generate (elements, testing::random::value<int>{});

    {
      auto const mspan {client.attach_mutable (chunk)};
      static_assert (std::is_same_v<decltype (mspan.data()), void*>);
      ASSERT_TRUE (mspan.ok());
      ASSERT_EQ (mspan.size(), size);
      auto span
        { std::span { util::cast<Element*> (mspan.data())
                    , number_of_elements
                    }
        };
      std::ranges::copy (elements, std::begin (span));
    }

    {
      auto const chunk_description
        {std::string {chunk.text(), chunk.text_size()}};
      auto const cspan
        { client.attach_const
            ( shmem_client::Chunk { chunk_description.data()
                                  , chunk_description.size()
                                  }
            )
        };
      static_assert (std::is_same_v<decltype (cspan.data()), void const*>);
      ASSERT_TRUE (cspan.ok());
      ASSERT_EQ (cspan.size(), size);
      auto span
        { std::span { util::cast<Element const*> (cspan.data())
                    , number_of_elements
                    }
        };
      ASSERT_THAT (elements, ::testing::ElementsAreArray (span));
    }

    ASSERT_TRUE (client.remove (chunk).ok());

    auto const res {client.remove (chunk)};
    ASSERT_FALSE (res.ok());
    ASSERT_STREQ
      (res.error(), "rpc::error::HandlerError: Unknown id 'bi_0'");
  }
}
