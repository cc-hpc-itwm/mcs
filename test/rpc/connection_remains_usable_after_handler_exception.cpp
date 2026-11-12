// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/error/HandlerException.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <stdexcept>

namespace
{
  struct Handler
  {
    struct Ping
    {
      using Response = int;

      int value;
    };

    struct Throw
    {
      using Response = void;

      int value;
    };

    auto operator() (Ping ping) const noexcept -> Ping::Response
    {
      return ping.value;
    }

    auto operator() (Throw request) const -> void
    {
      throw std::runtime_error
        { fmt::format ("expected handler failure: {}", request.value)
        };
    }
  };

  using Dispatcher = mcs::rpc::Dispatcher
                   < Handler
                   , Handler::Ping
                   , Handler::Throw
                   >;

  template<mcs::rpc::is_protocol P, mcs::rpc::is_access_policy AP>
    struct ProtocolAndAccessPolicy
  {
    using Protocol = P;
    using AccessPolicy = AP;
  };

  using ProtocolsAndAccessPolicies = ::testing::Types
    < ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , ProtocolAndAccessPolicy<asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , ProtocolAndAccessPolicy<asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    >;

  template<class>
    struct ConnectionAfterHandlerException : public mcs::testing::random::Test
  {};

  TYPED_TEST_SUITE
    ( ConnectionAfterHandlerException
    , ProtocolsAndAccessPolicies
    );
}

TYPED_TEST ( ConnectionAfterHandlerException
           , connection_remains_usable_after_handler_exception
           )
{
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;

  auto io_context
    { mcs::rpc::ScopedRunningIOContext
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
        { std::invoke (mcs::testing::random::value<unsigned int> {1u, 8u})
        }
      }
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
      ( {}
      , io_context
      )
    };
  auto const client
    { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
      ( io_context
      , provider.local_endpoint()
      )
    };

  auto const number_of_iterations
    { std::invoke (mcs::testing::random::value<unsigned int> {1u, 100u})
    };
  auto random_value {mcs::testing::random::value<int>{}};

  for (auto i {0u}; i < number_of_iterations; ++i)
  {
    {
      auto const value {std::invoke (random_value)};

      EXPECT_EQ (client (Handler::Ping {value}), value);
    }

    {
      auto const value {std::invoke (random_value)};

      mcs::testing::require_exception
        ( [&]
          {
            client (Handler::Throw {value});
          }
        , mcs::testing::assert_type_and_what<mcs::rpc::error::HandlerError>
            ( fmt::format
              ( "rpc::error::HandlerError: expected handler failure: {}"
              , value
              )
            )
        );
    }

    {
      auto const value {std::invoke (random_value)};

      EXPECT_EQ (client (Handler::Ping {value}), value);
    }
  }
}
