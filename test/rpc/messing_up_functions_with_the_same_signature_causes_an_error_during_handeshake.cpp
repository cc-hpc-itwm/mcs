// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/detail/make_handshake_data.hpp>
#include <mcs/rpc/error/HandshakeFailed.hpp>
#include <mcs/testing/require_exception.hpp>

namespace
{
  struct Handler
  {
    struct A { using Response = int; };
    struct B { using Response = int; };

    constexpr auto operator() (A) noexcept -> A::Response { return 0; }
    constexpr auto operator() (B) noexcept -> B::Response { return 1; }
  };

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class> struct RPCClientT : public ::testing::Test{};
  TYPED_TEST_SUITE (RPCClientT, Protocols);
}

TYPED_TEST ( RPCClientT
           , messing_up_functions_with_the_same_signature_causes_an_error_during_handshake
           )
{
  using Protocol = TypeParam;
  using DispatcherAB = mcs::rpc::Dispatcher<Handler, Handler::A, Handler::B>;
  using DispatcherBA = mcs::rpc::Dispatcher<Handler, Handler::B, Handler::A>;

  auto io_context_server
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };
  auto io_context_client
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, DispatcherAB> ({}, io_context_server)
    };

  using AccessPolicy = mcs::rpc::access_policy::Exclusive;

  mcs::testing::require_exception
    ( [&]
      {
        auto const client
          { mcs::rpc::make_client<Protocol, DispatcherBA, AccessPolicy>
              ( io_context_client
              , provider.local_endpoint()
              )
          };
      }
    , mcs::testing::assert_type_or_derived_type<mcs::rpc::error::HandshakeFailed>()
    , mcs::testing::assert_type_and_what<std::runtime_error>
        ( fmt::format
           ( "Not a prefix. Server: {}, Client: {}"
           , mcs::rpc::detail::make_handshake_data<Handler::A, Handler::B>()
           , mcs::rpc::detail::make_handshake_data<Handler::B, Handler::A>()
           )
        )
    );
}
