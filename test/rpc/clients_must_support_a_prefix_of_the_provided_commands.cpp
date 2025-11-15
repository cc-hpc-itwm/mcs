// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/detail/make_handshake_data.hpp>
#include <mcs/rpc/error/HandshakeFailed.hpp>
#include <mcs/testing/require_exception.hpp>

namespace
{
  struct A { using Response = void; };
  struct B { using Response = void; };
  struct C { using Response = void; };

  struct Handler
  {
    constexpr auto operator() (A) const noexcept -> A::Response{}
    constexpr auto operator() (B) const noexcept -> B::Response{}
    constexpr auto operator() (C) const noexcept -> C::Response{}
  };

  template<mcs::rpc::is_protocol P, mcs::rpc::is_access_policy AP>
    struct ProtocolAndAccessPolicy
  {
    using Protocol = P;
    using AccessPolicy = AP;
  };

  using ProtocolsAndAccessPolicies = ::testing::Types
    < ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>

    , ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>

    , ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    >;
  template<class> struct RPCClientT : public ::testing::Test{};
  TYPED_TEST_SUITE (RPCClientT, ProtocolsAndAccessPolicies);
}

TYPED_TEST ( RPCClientT
           , clients_must_support_a_prefix_of_the_provided_commands
           )
{
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;

  auto io_context_server
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };
  auto io_context_client
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };

  auto const provider
    { mcs::rpc::make_provider< Protocol
                        , mcs::rpc::Dispatcher<Handler, A, B, C>
                        > ({}, io_context_server)
    };

#define MCS_TEST_RPC_MAKE_CLIENT(_dispatcher...)                     \
  mcs::rpc::make_client<Protocol, _dispatcher, AccessPolicy>         \
    (io_context_client, provider.local_endpoint())

#define MCS_TEST_RPC_ASSERT_WORKS(_types...)                         \
  std::ignore =                                                      \
    MCS_TEST_RPC_MAKE_CLIENT (mcs::rpc::Dispatcher<Handler, _types>)

#define MCS_TEST_RPC_ASSERT_FAILS(_types...)                         \
    mcs::testing::require_exception                                  \
      ( [&]                                                          \
        {                                                            \
          std::ignore = MCS_TEST_RPC_MAKE_CLIENT                     \
            ( mcs::rpc::Dispatcher<Handler, _types>                  \
            )                                                        \
            ;                                                        \
        }                                                            \
      , mcs::testing::assert_type_or_derived_type                    \
          <mcs::rpc::error::HandshakeFailed>()                       \
      , mcs::testing::assert_type_and_what<std::runtime_error>       \
          ( fmt::format                                              \
              ( "Not a prefix. Server: {}, Client: {}"               \
              , mcs::rpc::detail::make_handshake_data<A, B, C>()     \
              , mcs::rpc::detail::make_handshake_data<_types>()      \
              )                                                      \
          )                                                          \
      )

  std::ignore = MCS_TEST_RPC_MAKE_CLIENT (mcs::rpc::Dispatcher<Handler>);
  MCS_TEST_RPC_ASSERT_WORKS (A);
  MCS_TEST_RPC_ASSERT_WORKS (A, B);
  MCS_TEST_RPC_ASSERT_WORKS (A, B, C);

  MCS_TEST_RPC_ASSERT_FAILS (B);
  MCS_TEST_RPC_ASSERT_FAILS (C);

  MCS_TEST_RPC_ASSERT_FAILS (A, C);
  MCS_TEST_RPC_ASSERT_FAILS (B, A);
  MCS_TEST_RPC_ASSERT_FAILS (B, C);
  MCS_TEST_RPC_ASSERT_FAILS (C, A);
  MCS_TEST_RPC_ASSERT_FAILS (C, B);

  MCS_TEST_RPC_ASSERT_FAILS (A, C, B);
  MCS_TEST_RPC_ASSERT_FAILS (B, A, C);
  MCS_TEST_RPC_ASSERT_FAILS (B, C, A);
  MCS_TEST_RPC_ASSERT_FAILS (C, A, B);
  MCS_TEST_RPC_ASSERT_FAILS (C, B, A);

  MCS_TEST_RPC_ASSERT_FAILS (A, B, C, A);
  MCS_TEST_RPC_ASSERT_FAILS (A, B, C, B);
  MCS_TEST_RPC_ASSERT_FAILS (A, B, C, C);

#undef MCS_TEST_RPC_ASSERT_FAILS
#undef MCS_TEST_RPC_ASSERT_WORKS
#undef MCS_TEST_RPC_MAKE_CLIENT
}
