// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <cstring>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/syscall/getrlimit.hpp>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{
  struct Handler
  {
    struct Inc { int value; using Response = int; };

    auto operator() (Inc i) noexcept -> Inc::Response
    {
      return++ i.value;
    }
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
  template<class> struct RPCClientT : public mcs::testing::random::Test{};
  TYPED_TEST_SUITE (RPCClientT, ProtocolsAndAccessPolicies);
}

TYPED_TEST ( RPCClientT
           , a_provider_can_be_used_by_more_clients_than_max_open_files
           )
{
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;
  using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Inc>;

  auto io_context_server
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };
  auto io_context_client
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher> ({}, io_context_server)
    };

  auto const max_open_files
    { std::invoke
      ( []
        {
          try
          {
            return mcs::util::syscall::getrlimit (RLIMIT_NOFILE).rlim_cur;
          }
          catch (...)
          {
            std::throw_with_nested
              (std::runtime_error {"Could not determine max_open_file."});
          }
        }
      )
    };

  // use some threads to speed up the test
  std::vector<std::future<int>> errors;
  auto const num_threads {std::thread::hardware_concurrency()};
  auto const divru
    { [] (auto a, auto b)
      {
        return (a + b - 1) / b;
      }
    };
  auto const clients_per_thread {divru (max_open_files + 1, num_threads)};

  auto random_value {mcs::testing::random::value<int>{}};

  for (auto t {0u}; t < num_threads; ++t)
  {
    errors.emplace_back
      ( std::async
        ( std::launch::async
        , [&]
          {
            auto e {0};

            for (auto i {0u}; i < clients_per_thread; ++i)
            {
              auto const client
                { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
                  ( io_context_client
                  , provider.local_endpoint()
                  )
                };

              auto const value {random_value()};
              if (client (Handler::Inc {value}) != value + 1)
              {
                ++e;
              }
            }

            return e;
          }
        )
      );
  }

  for (auto& error : errors)
  {
    ASSERT_EQ (error.get(), 0);
  }
}
