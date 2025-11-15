// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#include <compare>
#include <functional>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <type_traits>

namespace
{
  struct Command
  {
    constexpr Command() noexcept = delete;
    constexpr explicit Command (int value) noexcept : _value {value} {}
    explicit Command (mcs::serialization::IArchive& ia)
      : _value {mcs::serialization::load<decltype (_value)> (ia)}
    {}
    auto save (mcs::serialization::OArchive& oa) const -> mcs::serialization::OArchive&
    {
      return mcs::serialization::save (oa, _value);
    }
    Command (Command const&) = delete;
    Command (Command&&) noexcept = default;
    auto operator= (Command const&) -> Command& = delete;
    auto operator= (Command&&) noexcept -> Command& = default;
    ~Command() noexcept = default;

    struct Response
    {
      constexpr Response() noexcept = delete;
      constexpr explicit Response (int value) noexcept : _value {value} {}
      explicit Response (mcs::serialization::IArchive& ia)
        : _value {mcs::serialization::load<decltype (_value)> (ia)}
      {}
      auto save (mcs::serialization::OArchive& oa) const -> mcs::serialization::OArchive&
      {
        return mcs::serialization::save (oa, _value);
      }
      Response (Response const&) = delete;
      Response (Response&&) noexcept = default;
      auto operator= (Response const&) -> Response& = delete;
      auto operator= (Response&&) noexcept -> Response& = default;
      ~Response() noexcept = default;

      int _value;

      constexpr auto operator<=> (Response const&) const noexcept = default;
    };

    int _value;
  };
  static_assert (!std::is_default_constructible_v<Command>);
  static_assert (!std::is_default_constructible_v<Command::Response>);
  static_assert (!std::is_copy_constructible_v<Command>);
  static_assert (!std::is_copy_constructible_v<Command::Response>);
  static_assert (!std::is_copy_assignable_v<Command>);
  static_assert (!std::is_copy_assignable_v<Command::Response>);

  struct Inc
  {
    constexpr explicit Inc (int offset) noexcept
      : _offset {offset}
    {}
    [[nodiscard]] constexpr auto inc
      ( Command const& command
      ) const noexcept -> Command::Response
    {
      return Command::Response {command._value + _offset};
    }
  private:
    int _offset;
  };

  struct HandleByValueResponse : public Inc
  {
    using Inc::Inc;
    auto operator() (Command command) const noexcept -> Command::Response
    {
      return Inc::inc (command);
    }
  };
  struct HandleByValueAwaitableResponse : public Inc
  {
    using Inc::Inc;
    auto operator()
      ( Command command
      ) const -> asio::awaitable<Command::Response>
    {
      co_return Inc::inc (command);
    }
  };

  struct HandleByConstRefResponse : public Inc
  {
    using Inc::Inc;
    auto operator()
      ( Command const& command
      ) const noexcept -> Command::Response
    {
      return Inc::inc (command);
    }
  };
  struct HandleByConstRefAwaitableResponse : public Inc
  {
    using Inc::Inc;
    auto operator()
      ( Command const& command
      ) const -> asio::awaitable<Command::Response>
    {
      co_return Inc::inc (command);
    }
  };

  struct HandleByUniversalRefResponse : public Inc
  {
    using Inc::Inc;
    auto operator() (Command&& command) const noexcept -> Command::Response
    {
      return Inc::inc (command);
    }
  };
  struct HandleByUniversalRefAwaitableResponse : public Inc
  {
    using Inc::Inc;
    auto operator()
      ( Command&& command
      ) const -> asio::awaitable<Command::Response>
    {
      co_return Inc::inc (command);
    }
  };

  template<typename H, mcs::rpc::is_protocol P, mcs::rpc::is_access_policy AP>
    struct HandlerAndProtocolAndAccessPolicy
  {
    using Handler = H;
    using Protocol = P;
    using AccessPolicy = AP;
  };

  using HandlersAndProtocolsAndAccessPolicies = ::testing::Types
    < HandlerAndProtocolAndAccessPolicy<HandleByValueResponse                , asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByValueAwaitableResponse       , asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefResponse             , asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefAwaitableResponse    , asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefResponse         , asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefAwaitableResponse, asio::ip::tcp               , mcs::rpc::access_policy::Exclusive>

    , HandlerAndProtocolAndAccessPolicy<HandleByValueResponse                , asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByValueAwaitableResponse       , asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefResponse             , asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefAwaitableResponse    , asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefResponse         , asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefAwaitableResponse, asio::local::stream_protocol, mcs::rpc::access_policy::Exclusive>

    , HandlerAndProtocolAndAccessPolicy<HandleByValueResponse                , asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByValueAwaitableResponse       , asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefResponse             , asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefAwaitableResponse    , asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefResponse         , asio::ip::tcp               , mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefAwaitableResponse, asio::ip::tcp               , mcs::rpc::access_policy::Sequential>

    , HandlerAndProtocolAndAccessPolicy<HandleByValueResponse                , asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByValueAwaitableResponse       , asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefResponse             , asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefAwaitableResponse    , asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefResponse         , asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefAwaitableResponse, asio::local::stream_protocol, mcs::rpc::access_policy::Sequential>

    , HandlerAndProtocolAndAccessPolicy<HandleByValueResponse                , asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByValueAwaitableResponse       , asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefResponse             , asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefAwaitableResponse    , asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefResponse         , asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefAwaitableResponse, asio::ip::tcp               , mcs::rpc::access_policy::Concurrent>

    , HandlerAndProtocolAndAccessPolicy<HandleByValueResponse                , asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByValueAwaitableResponse       , asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefResponse             , asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByConstRefAwaitableResponse    , asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefResponse         , asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    , HandlerAndProtocolAndAccessPolicy<HandleByUniversalRefAwaitableResponse, asio::local::stream_protocol, mcs::rpc::access_policy::Concurrent>
    >
    ;
  template<class> struct RPCClientT : mcs::testing::random::Test{};
  TYPED_TEST_SUITE (RPCClientT, HandlersAndProtocolsAndAccessPolicies);
}

TYPED_TEST ( RPCClientT
           , no_default_ctor_command_and_response_works_for_all_call_policies
           )
{
  using Handler = typename TypeParam::Handler;
  using Protocol = typename TypeParam::Protocol;
  using AccessPolicy = typename TypeParam::AccessPolicy;
  using Dispatcher = mcs::rpc::Dispatcher<Handler, Command>;

  auto random_value {mcs::testing::random::value<int>{}};

  auto io_context_server
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };
  auto io_context_client
    { mcs::rpc::ScopedRunningIOContext
        {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}}
    };

  auto const offset {random_value()};

  auto const provider
    { mcs::rpc::make_provider<Protocol, Dispatcher> ({}, io_context_server, offset)
    };

  auto const client
    { mcs::rpc::make_client<Protocol, Dispatcher, AccessPolicy>
        ( io_context_client
        , provider.local_endpoint()
        )
    };

  auto const value {random_value()};
  auto const command {Command {value}};
  auto const expected {Handler {offset}.inc (command)};

  ASSERT_EQ (client (Command {value}), expected);
  ASSERT_EQ (client (std::cref (command)), expected);
  ASSERT_EQ (client.template call<Command> (value), expected);

  ASSERT_EQ (client.get_future (Command {value}).get(), expected);
  ASSERT_EQ (client.get_future (std::cref (command)).get(), expected);
  ASSERT_EQ (client.template async_call<Command> (value).get(), expected);
}
