// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <map>
#include <mcs/rpc/ASyncRemoteFunction.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/SyncRemoteFunction.hpp>
#include <mcs/rpc/error/HandlerException.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/not_null.hpp>
#include <optional>
#include <stdexcept>
#include <tuple>

namespace
{
  namespace kvs
  {
    struct Assign
    {
      int key {0};
      int value {0};

      struct Response
      {
        std::optional<int> old_value;
      };
    };

    struct Retrieve
    {
      int key;

      using Response = std::optional<int>;
    };

    struct At
    {
      int key;

      using Response = int;
    };

    struct Increment
    {
      int key;

      using Response = int;
    };
  }

  namespace client
  {
    struct N
    {
      using Response = unsigned int;
    };
  }

  // shared between clients
  struct KVS
  {
    auto operator() (kvs::Assign assign)
    {
      auto res {_assignments.emplace (assign.key, assign.value)};

      auto response {decltype (assign)::Response{}};

      if (!res.second)
      {
        response.old_value = res.first->second;
      }

      return response;
    }

    auto operator() (kvs::Retrieve retrieve)
    {
      auto res {_assignments.find (retrieve.key)};

      return res != std::cend (_assignments)
        ? decltype (retrieve)::Response {res->second}
      : decltype (retrieve)::Response{}
      ;
    }

    auto operator() (kvs::At at)
    {
      return decltype (at)::Response {_assignments.at (at.key)};
    }

    auto operator() (kvs::Increment inc)
    {
      return decltype (inc)::Response {++_assignments.at (inc.key)};
    }

  private:
    //! \note server is single-threaded -> no lock is required
    std::map<int, int> _assignments;
  };

  struct KVSHandler
  {
    KVSHandler (mcs::util::not_null<KVS> kvs) : _kvs {kvs} {}
    mcs::util::not_null<KVS> _kvs;

    auto operator() (kvs::Assign assign)
    {
      ++_n;
      return _kvs->operator() (assign);
    }
    auto operator() (kvs::Retrieve retrieve)
    {
      ++_n;
      return _kvs->operator() (retrieve);
    }
    auto operator() (kvs::At at)
    {
      ++_n;
      return _kvs->operator() (at);
    }
    auto operator() (kvs::Increment inc)
    {
      ++_n;
      return _kvs->operator() (inc);
    }
    auto operator() (client::N) const noexcept
    {
      return _n;
    }

    // client specific
    unsigned int _n {0};
  };

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class> struct RPCClientT : public mcs::testing::random::Test{};
  TYPED_TEST_SUITE (RPCClientT, Protocols);
}

TYPED_TEST (RPCClientT, synchronous_call_basics)
{
  using Protocol = TypeParam;
  using kvs::Assign;
  using kvs::Retrieve;
  using kvs::At;
  using kvs::Increment;
  using client::N;

  using Dispatcher = mcs::rpc::Dispatcher< KVSHandler
                                    , Assign
                                    , Retrieve
                                    , At
                                    , Increment
                                    , N
                                    >;

  auto io_context
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };

  auto kvs {KVS{}};
  auto random_value {mcs::testing::random::value<int>{}};

  auto const key {random_value()};
  auto const value {random_value()};

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher>
         ( {}
         , io_context
         , std::addressof (kvs)
         )
    };

  auto const client1
    { mcs::rpc::make_client<Protocol, Dispatcher, mcs::rpc::access_policy::Exclusive>
        ( io_context
        , provider.local_endpoint()
        )
    };

  // deliver the command as rvalue, forwarded into the client
  ASSERT_FALSE (client1 (Assign {key, value}).old_value.has_value());

  // deliver command by constructor arguments, create in the client
  ASSERT_TRUE
    (client1.template call<Assign> (key, random_value()).old_value.has_value());

  {
    auto const old {client1 (Assign {key, random_value()}).old_value};

    ASSERT_TRUE (old.has_value() && (old.value() == value));
  }

  // create a function to call with the constructor arguments of the
  // command
  auto retrieve {make_sync_remote_function<Retrieve> (client1)};
  auto retrieve_key {Retrieve {key}};

  ASSERT_TRUE (retrieve (Retrieve {key}).has_value());
  {
    auto val {retrieve (std::cref (retrieve_key))};

    ASSERT_TRUE (val.has_value() && (val.value() == value));
  }
  ASSERT_FALSE (retrieve (key + 1).has_value());
  ASSERT_EQ (client1 (At {key}), value);
  mcs::testing::require_exception
    ( [&]
      {
        std::ignore = client1 (At {key + 1});
      }
    , mcs::testing::assert_type<mcs::rpc::error::HandlerError>()
    );
  ASSERT_EQ (client1 (Increment {key}), value + 1);

  // deliver the command as reference, make sure it outlives the call
  {
    auto const command_N {N{}};

    ASSERT_EQ (client1 (std::cref (command_N)), 9u);
    // calls to N do not count
    ASSERT_EQ (client1 (std::cref (command_N)), 9u);
  }

  auto const client2
    { mcs::rpc::make_client<Protocol, Dispatcher, mcs::rpc::access_policy::Exclusive>
        ( io_context
        , provider.local_endpoint()
        )
    };

  ASSERT_EQ (client2 (N{}), 0u);

  // async_calls for ExclusiveAccess clients are never blocking
  auto increment {make_async_remote_function<Increment> (client1)};

  auto i2 {client2.template async_call<Increment> (key)};
  auto i1 {increment (key)};

  auto v1 {i1.get()};
  auto v2 {i2.get()};

  // the order of the async calls is not defined
  ASSERT_GE (v1, value + 2);
  ASSERT_LE (v1, value + 3);
  ASSERT_GE (v2, value + 2);
  ASSERT_LE (v2, value + 3);
  ASSERT_NE (v1, v2);
}
