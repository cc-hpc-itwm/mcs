// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/use_future.hpp>
#include <chrono>
#include <exception>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <map>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>
#include <mcs/rpc/multi_client/call.hpp>
#include <mcs/rpc/multi_client/call_and_collect.hpp>
#include <mcs/rpc/multi_client/command_generator/Const.hpp>
#include <mcs/rpc/multi_client/command_generator/Range.hpp>
#include <mcs/rpc/multi_client/command_generator/Reference.hpp>
#include <mcs/rpc/multi_client/command_generator/Sequence.hpp>
#include <mcs/testing/printer/STD/exception.hpp>
#include <mcs/testing/printer/STD/list.hpp>
#include <mcs/testing/printer/STD/tuple.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/TaggedRange.hpp>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace
{
  using RandomUInt = mcs::testing::random::value<unsigned int>;

  struct Ping
  {
    unsigned int value;

    using Response = unsigned int;
  };

  struct Handler
  {
    // 1-2 ms
    mcs::testing::random::value<unsigned long> micros {1000ul, 2000ul};

    // For the tests below the handler produces
    // - an exception on value 42u
    // - a much slower response for 43u that has a high(er) probablity
    //   to be still in progress after the exception has ended
    //   throwing calls
    auto operator() (Ping ping) -> asio::awaitable<typename Ping::Response>
    {
      auto timer {asio::steady_timer {co_await asio::this_coro::executor}};
      timer.expires_after ( ping.value == 43u
                          ? std::chrono::microseconds {micros() * 100u}
                          : std::chrono::microseconds {micros()}
                          );
      co_await timer.async_wait (asio::use_awaitable);

      if (ping.value == 42u)
      {
        throw std::runtime_error {"nope, too many mouse"};
      }

      co_return ping.value;
    }
  };

  // collectors
  template<typename Command>
    struct CollectResultsAndThrowErrors
  {
    using Result = typename Command::Response;
    using Results = std::list<Result>;

    Results results;

    template<typename ClientID>
      auto result (ClientID const&, Result result)
    {
      results.push_back (std::move (result));
    }
    // to omit an error collector makes the rpc to throw the error
  };

  template<typename Command>
    struct CollectResultsAndIgnoreErrors
  {
    using Result = typename Command::Response;
    using Results = std::list<Result>;

    Results results;

    template<typename ClientID>
      auto result (ClientID const&, Result result)
    {
      results.push_back (std::move (result));
    }
    template<typename ClientID>
      auto error (ClientID const&, std::exception_ptr)
    {
      // ignore errors
    }
  };

  template<typename Command>
    struct CollectResultsAndCollectErrors
  {
    using Result = typename Command::Response;
    using Results = std::list<Result>;
    using Errors = std::list<std::exception_ptr>;

    Results results;
    Errors errors;

    template<typename ClientID>
      auto result (ClientID const&, Result result)
    {
      results.push_back (std::move (result));
    }
    template<typename ClientID>
      auto error (ClientID const&, std::exception_ptr error)
    {
      errors.emplace_back (error);
    }
  };

  struct ResultsSum
  {
    template<typename ClientID>
      auto result (ClientID const&, Ping::Response result) noexcept
    {
      _values.emplace (result);

      if (_sum.has_value())
      {
        *_sum += result;
      }
      else
      {
        _sum = result;
      }
    }

    std::optional<Ping::Response> _sum;
    std::unordered_multiset<Ping::Response> _values;
  };

  struct NoThrowResultsSumOfSuccesses
  {
    template<typename ClientID>
      auto result (ClientID const&, Ping::Response result) noexcept
    {
      if (_sum.has_value())
      {
        *_sum += result;
      }
      else
      {
        _sum = result;
      }
    }
    template<typename ClientID>
      auto error (ClientID const&, std::exception_ptr) noexcept
    {
      // ignore error, just sum up the successful runs
    }

    std::optional<Ping::Response> _sum;
  };

  using PingDispatcher = mcs::rpc::Dispatcher<Handler, Ping>;

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
}

namespace mcs::rpc
{
  namespace
  {
    template<is_protocol Protocol>
      struct Server
    {
      [[nodiscard]] constexpr auto local_endpoint() const
      {
        return _provider.local_endpoint();
      }

    private:
      ScopedRunningIOContext _io_context
        {ScopedRunningIOContext::NumberOfThreads {1u}, SIGTERM, SIGINT};

      Provider<Protocol, PingDispatcher> _provider
        {make_provider<Protocol, PingDispatcher> ({}, _io_context)};
    };
  }

  namespace
  {
    template<rpc::is_protocol Protocol>
      struct RPCMultiClientT : public testing::random::Test
    {
      ScopedRunningIOContext io_context
        { ScopedRunningIOContext::NumberOfThreads {RandomUInt {1u, 8u}()}
        , SIGTERM
        , SIGINT
        };

      template<is_access_policy AccessPolicy>
        auto make_clients (std::list<Server<Protocol>> const& servers)
      {
        using ClientType
          = typename PingDispatcher::ClientType<Protocol, AccessPolicy>
          ;

        auto cs {std::list<std::variant<ClientType>>{}};

        for (auto const& server : servers)
        {
          cs.emplace_back
            ( make_client<Protocol, PingDispatcher, AccessPolicy>
                (io_context, server.local_endpoint())
            );
        }

        return cs;
      }
    };
    TYPED_TEST_SUITE (RPCMultiClientT, Protocols);
  }

  TYPED_TEST (RPCMultiClientT, basics)
  {
    using Protocol = TypeParam;

    auto const number_of_servers {10u};
    auto const parallel_calls_limits
      { [&]() -> multi_client::ParallelCallsLimit
        {
          auto const limit {RandomUInt {1u, 5u * number_of_servers / 4u}()};

          if (limit < number_of_servers)
          {
            // limited to some number smaller than the number of
            // servers with probablity 80%
            return multi_client::ParallelCalls::AtMost {limit};
          }
          else
          {
            // unlimited number of parallel calls with probablity 20%
            return multi_client::ParallelCalls::Unlimited{};
          }
        }()
      };

    auto servers {std::list<Server<Protocol>> (number_of_servers)};
    auto clients
      {this->template make_clients<access_policy::Sequential> (servers)};

    // usage of call to create a collector on the fly
    {
      auto expected_results
        {typename CollectResultsAndIgnoreErrors<Ping>::Results{}};

      auto const initial_value {10u};
      {
        auto value {multi_client::command_generator::Sequence {initial_value}};

        for (auto const& client : clients)
        {
          expected_results.emplace_back (value (client));
        }
      }

      auto const collected
        { multi_client::call<CollectResultsAndIgnoreErrors, Ping>
            ( multi_client::command_generator::Sequence {initial_value}
            , clients
            , parallel_calls_limits
            )
        };

      // \note The completion order is unspecified: Later calls may
      // overtake earlier calls.
      //
      ASSERT_THAT ( collected.results
                  , ::testing::UnorderedElementsAreArray (expected_results)
                  );
   }

    // getting back errors via and ensure all calls are executed
    {
      auto expected_results
        {typename CollectResultsAndCollectErrors<Ping>::Results{}};

      auto const initial_value {40u};
      {
        auto value {multi_client::command_generator::Sequence {initial_value}};

        for (auto const& client : clients)
        {
          if (auto const val {value (client)}; val != 42u)
          {
            expected_results.emplace_back (val);
          }
        }
      }

      auto const collected
        { multi_client::call<CollectResultsAndCollectErrors, Ping>
            ( multi_client::command_generator::Sequence {initial_value}
            , clients
            , parallel_calls_limits
            )
        };

      ASSERT_THAT ( collected.results
                  , ::testing::UnorderedElementsAreArray (expected_results)
                  );

      ASSERT_EQ (collected.errors.size(), 1);
      for (auto const& error : collected.errors)
      {
        testing::require_exception
          ( [&]
            {
              std::rethrow_exception (error);
            }
          , testing::assert_type<error::HandlerError>()
          );
      }
    }

    // using a throwing collector re-throws errors
    testing::require_exception
      ( [&]
        {
          multi_client::call<CollectResultsAndThrowErrors, Ping>
            ( multi_client::command_generator::Sequence {40u}
            , clients
            , parallel_calls_limits
            );
        }
      , testing::assert_type<multi_client::Errors>()
      );

    // drop the client and the connection to the first server to
    // demonstrate that the clients are independent from the
    // multi-client
    clients.pop_front();

    // collectors can be used in multiple (sequential) calls to
    // call_and_collect
    // \note: there is no synchronization between the calls, if the
    // collector is to be used in multiple parallel calls, then it
    // must synchronize internally
    {
      auto collector {ResultsSum{}};

      auto const N {RandomUInt {2u, 100u}()};

      for (auto i {0u}; i < N; ++i)
      {
        multi_client::call_and_collect<Ping>
          ( multi_client::command_generator::Sequence {1u}
          , collector
          , clients
          , parallel_calls_limits
          );

        ASSERT_EQ ( collector._sum
                  , std::optional<unsigned int>
                    {(i + 1u) * (1u + 2u + 3u + 4u + 5u + 6u + 7u + 8u + 9u)}
                  );
      }
    }

    // stateful call_and_collect stops collecting after an error
    // occured and delivers all results that have been produced so
    // far. That might include results from later calls.
    {
      auto collector {ResultsSum{}};
      auto const initial_value {40u};

      testing::require_exception
        ( [&]
          {
            multi_client::call_and_collect<Ping>
              ( multi_client::command_generator::Sequence {initial_value}
              , collector
              , clients
              , parallel_calls_limits
              );
          }
        , testing::assert_type<multi_client::Errors>()
        );

      for (auto result : collector._values)
      {
        ASSERT_GE (result, initial_value);
        ASSERT_NE (result, 42u);
        ASSERT_LT (result, initial_value + clients.size());
      }
    }

    // stateful call_and_collect can ignore errors, too
    {
      auto collector {NoThrowResultsSumOfSuccesses{}};

      multi_client::call_and_collect<Ping>
        ( multi_client::command_generator::Sequence {40u}
        , collector
        , clients
        , parallel_calls_limits
        );

      ASSERT_EQ ( collector._sum
                , std::optional<unsigned int>
                  {40u + 41u + 43u + 44u + 45u + 46u + 47u + 48u}
                );
    }

    // stateful call_and_collect without errors collects all results
    {
      auto collector {NoThrowResultsSumOfSuccesses{}};

      multi_client::call_and_collect<Ping>
        ( multi_client::command_generator::Const<Ping> {1u}
        , collector
        , clients
        , parallel_calls_limits
        );

      ASSERT_EQ ( collector._sum
                , std::optional<unsigned int>
                  {1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u}
                );
    }

    // multiple parallel calls are possible, asio::co_spawn
    {
      auto call_io_context
        { ScopedRunningIOContext
            { ScopedRunningIOContext::NumberOfThreads {RandomUInt {1u, 8u}()}
            , SIGTERM, SIGINT
            }
        };

      auto const number_of_parallel_calls {RandomUInt {2u, 100u}()};

      auto collectors {std::list<NoThrowResultsSumOfSuccesses>{}};
      auto calls {std::list<std::future<void>>{}};

      for (auto i {0u}; i < number_of_parallel_calls; ++i)
      {
        auto& collector {collectors.emplace_back()};

        calls.emplace_back
          ( asio::co_spawn
            ( call_io_context
            , [&]() -> asio::awaitable<void>
              {
                multi_client::call_and_collect<Ping>
                  ( multi_client::command_generator::Const<Ping> {1u}
                  , collector
                  , clients
                  , parallel_calls_limits
                  );

                co_return;
              }
            , asio::use_future
            )
          );
      }

      // \todo ensure the calls are running in parallel

      for (auto& call : calls)
      {
        call.get();
      }
      for (auto const& collector : collectors)
      {
        EXPECT_EQ ( collector._sum
                  , std::optional<unsigned int>
                    {1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u}
                  );
      }
    }

    // multiple parallel calls are possible, std::async
    {
      auto const number_of_parallel_calls {RandomUInt {2u, 100u}()};

      auto collectors {std::list<NoThrowResultsSumOfSuccesses>{}};
      auto calls {std::list<std::future<void>>{}};

      for (auto i {0u}; i < number_of_parallel_calls; ++i)
      {
        auto& collector {collectors.emplace_back()};

        calls.emplace_back
          ( std::async
            ( std::launch::async
            , [&]
              {
                multi_client::call_and_collect<Ping>
                  ( multi_client::command_generator::Const<Ping> {1u}
                  , collector
                  , clients
                  , parallel_calls_limits
                  );
              }
            )
          );
      }

      // \todo ensure the calls are running in parallel

      for (auto& call : calls)
      {
        call.get();
      }
      for (auto const& collector : collectors)
      {
        ASSERT_EQ ( collector._sum
                  , std::optional<unsigned int>
                    {1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u}
                  );
      }
    }

    // re-establish the connection to the first server, again to
    // demonstrate that the clients are independent from the
    // multi-client
    clients.emplace_back
      ( make_client<Protocol, PingDispatcher, access_policy::Sequential>
          (this->io_context, servers.front().local_endpoint())
      );

    // usage of a reference to a command, no copies before serialization
    {
      auto collector {NoThrowResultsSumOfSuccesses{}};
      auto const ping {Ping {1u}};

      multi_client::call_and_collect<Ping>
        ( multi_client::command_generator::Reference<Ping> {ping}
        , collector
        , clients
        , parallel_calls_limits
        );

      ASSERT_EQ ( collector._sum
                , std::optional<unsigned int>
                  {1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u + 1u}
                );
    }

    // multiple clients targeting the same server are fine
    {
      // \note collectors have no way to distinguish between the clients
      // that produced a result, the collectors are just presented with
      // the endpoint and in case of duplicates the same endpoint shows
      // up multiple time (with potential different results!)
      clients.emplace_back
        ( make_client<Protocol, PingDispatcher, access_policy::Sequential>
           (this->io_context, servers.front().local_endpoint())
        );

      auto collector {NoThrowResultsSumOfSuccesses{}};

      multi_client::call_and_collect<Ping>
        ( multi_client::command_generator::Sequence {1u}
        , collector
        , clients
        , parallel_calls_limits
        );

      ASSERT_EQ ( collector._sum
                , std::optional<unsigned int>
                  {1u + 2u + 3u + 4u + 5u + 6u + 7u + 8u + 9u + 10u + 11u}
                );
    }

    // the range generator can be used for any kind of range
    {
      auto collector {NoThrowResultsSumOfSuccesses{}};

      auto inputs {std::list<unsigned int>{}};

      std::generate_n
        ( std::back_inserter (inputs)
        , clients.size()
        , testing::random::value<unsigned int>{}
        );

      multi_client::call_and_collect<Ping>
        ( multi_client::command_generator::Range {inputs}
        , collector
        , clients
        , parallel_calls_limits
        );

      ASSERT_EQ
        ( collector._sum
        , std::optional<unsigned int>
            {std::accumulate (std::cbegin (inputs), std::cend (inputs), 0u)}
        );
    }
  }

  TYPED_TEST
    ( RPCMultiClientT
    , clients_can_be_delivered_indirectly
    )
  {
    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};
    auto clients_storage
      {this->template make_clients<access_policy::Sequential> (servers)};
    auto clients {util::make_unordered_tagged_range (0u, clients_storage)};

    auto collect {NoThrowResultsSumOfSuccesses{}};

    multi_client::call_and_collect<Ping>
      ( [] (auto const& tag) { return 100u + tag; }
      , [&] (auto tag) { return clients.at (tag); }
      , collect
      , clients.tags()
      , multi_client::ParallelCalls::Unlimited{}
      );

    auto const N {servers.size()};

    ASSERT_GT (N, 0u);

    ASSERT_EQ
      ( collect._sum
      , std::optional<unsigned int> {N * 100u + (N * (N - 1u)) / 2u}
      );
  }

  TYPED_TEST
    ( RPCMultiClientT
    , collector_knows_about_the_client_that_produced_the_result_or_error
    )
  {
    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};
    auto clients_storage
      {this->template make_clients<access_policy::Sequential> (servers)};
    auto clients {util::make_unordered_tagged_range (0u, clients_storage)};

    struct Collector
    {
      std::map<unsigned int, Ping::Response> results;
      std::map<unsigned int, std::exception_ptr> errors;

      auto result (unsigned int id, Ping::Response result)
      {
        results.emplace (id, std::move (result));
      }
      auto error (unsigned int id, std::exception_ptr error)
      {
        errors.emplace (id, error);
      }
    };

    auto command_generator {[] (auto const& tag) { return 40u + tag; }};
    auto collector {Collector{}};

    multi_client::call_and_collect<Ping>
      ( command_generator
      , [&] (auto tag) { return clients.at (tag); }
      , collector
      , clients.tags()
      , multi_client::ParallelCalls::Unlimited{}
      );

    for (auto tag : clients.tags())
    {
      if (command_generator (tag) == 42u)
      {
        ASSERT_TRUE (collector.errors.contains (tag));
      }
      else
      {
        ASSERT_TRUE (collector.results.contains (tag));
      }
    }
  }

  TYPED_TEST
    ( RPCMultiClientT
    , parallel_calls_limit_is_respected
    )
  {
    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};
    auto clients_storage
      {this->template make_clients<access_policy::Sequential> (servers)};
    auto clients {util::make_unordered_tagged_range (0u, clients_storage)};
    auto const parallel_calls_limit {RandomUInt {1u, 10u}()};

    auto guard {std::mutex{}};
    auto max_number_of_parallel_calls {0u};
    auto number_of_parallel_calls {0u};

    struct Collector
    {
      constexpr Collector
        ( std::mutex* guard
        , unsigned int* number_of_parallel_calls
        ) noexcept
          : _guard {guard}
          , _number_of_parallel_calls {number_of_parallel_calls}
      {}

      auto result (unsigned int, Ping::Response result)
      {
        if (_sum.has_value())
        {
          *_sum += result;
        }
        else
        {
          _sum = result;
        }

        auto const lock {std::lock_guard {*_guard}};
        if (*_number_of_parallel_calls == 0u)
        {
          throw std::logic_error {"call completed and no calls ongoing"};
        }
        --*_number_of_parallel_calls;
      }
      auto error (unsigned int, std::exception_ptr)
      {
        auto const lock {std::lock_guard {*_guard}};
        if (*_number_of_parallel_calls == 0u)
        {
          throw std::logic_error {"call completed and no calls ongoing"};
        }
        --*_number_of_parallel_calls;
      }

      std::optional<Ping::Response> _sum;

    private:
      std::mutex* _guard;
      unsigned int* _number_of_parallel_calls;
    };

    auto command_generator
      { [&] (auto const& tag)
        {
          {
            auto const lock {std::lock_guard {guard}};
            ++number_of_parallel_calls;
            max_number_of_parallel_calls = std::max
              ( max_number_of_parallel_calls
              , number_of_parallel_calls
              )
              ;
          }

          return 100u + tag;
        }
      };
    auto collect
      { Collector { std::addressof (guard)
                  , std::addressof (number_of_parallel_calls)
                  }
      };

    multi_client::call_and_collect<Ping>
      ( command_generator
      , [&] (auto tag) { return clients.at (tag); }
      , collect
      , clients.tags()
      , multi_client::ParallelCalls::AtMost {parallel_calls_limit}
      );

    auto const N {servers.size()};

    ASSERT_GT (N, 0u);

    ASSERT_EQ
      ( collect._sum
      , std::optional<unsigned int> {N * 100u + (N * (N - 1u)) / 2u}
      );

    ASSERT_GT (max_number_of_parallel_calls, 0u);
    ASSERT_LE (max_number_of_parallel_calls, parallel_calls_limit);
  }

  TYPED_TEST
    ( RPCMultiClientT
    , collect_can_throttle
    )
  {
    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};
    auto clients_storage
      {this->template make_clients<access_policy::Sequential> (servers)};
    auto clients {util::make_unordered_tagged_range (0u, clients_storage)};

    // do not generate all commands at once
    auto const parallel_calls_limit {RandomUInt {1u, 10u}()};

    struct ThrottlingCollector
    {
      constexpr auto result (unsigned int, Ping::Response result) noexcept
      {
        if (_sum.has_value())
        {
          *_sum += result;
        }
        else
        {
          _sum = result;
        }
      }
      auto error (unsigned int, std::exception_ptr) noexcept
      {
        ++_number_of_errors;
      }
      [[nodiscard]] constexpr auto start_more_calls() const noexcept
      {
        return _number_of_errors == 0u;
      }

      std::optional<Ping::Response> _sum;
      unsigned int _number_of_errors {0u};
    };

    auto expected_sum {0u};

    auto command_generator
      { [&] (auto const& tag)
        {
          auto value {40u + tag};

          if (value != 42u)
          {
            // every started command will complete
            expected_sum += value;
          }

          return value;
        }
      };
    auto collect {ThrottlingCollector{}};

    multi_client::call_and_collect<Ping>
      ( command_generator
      , [&] (auto tag) { return clients.at (tag); }
      , collect
      , clients.tags()
      , multi_client::ParallelCalls::AtMost {parallel_calls_limit}
      );

    ASSERT_EQ
      ( collect._sum
      , std::optional<unsigned int> {expected_sum}
      );

    auto const N {servers.size()};

    ASSERT_GT (N, 0u);

    // not all commands were generated
    ASSERT_LT
      ( collect._sum
      , std::optional<unsigned int> {N * 100u + (N * (N - 1u)) / 2u}
      );
  }

  TYPED_TEST
    ( RPCMultiClientT
    , drop_clients_while_call_might_be_ongoing_works
    )
  {
    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};

    auto collector {CollectResultsAndThrowErrors<Ping>{}};

    auto const initial_value {40u};
    testing::require_exception
      ( [&]
        {
          multi_client::call_and_collect<Ping>
            ( multi_client::command_generator::Sequence {initial_value}
            , collector
            , this->template make_clients<access_policy::Sequential> (servers)
            , multi_client::ParallelCalls::Unlimited{}
            );
        }
      , testing::assert_type<multi_client::Errors>()
      );
      // dropped clients

    // once call_and_collect has been completed, is it safe to access
    // the results

    // \note The results are collected in unspecified order. So there
    // might be results but it must not be the 42.
    for (auto result : collector.results)
    {
      ASSERT_GE (result, initial_value);
      ASSERT_NE (result, 42u);
      ASSERT_LT (result, initial_value + servers.size());
    }
  }

  TYPED_TEST
    ( RPCMultiClientT
    , access_policy_Exclusive_works_with_nothrow_collectors_and_separated_calls
    )
  {
    // \note The same test with a _throwing_ collector fails because
    // it would drop async operations, which is only allowed for the
    // access policy Sequential. See drop_async_operation, too.

    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};
    auto clients
      {this->template make_clients<access_policy::Exclusive> (servers)};

    auto const number_of_calls {RandomUInt {2u, 100u}()};

    for (auto i {0u}; i < number_of_calls; ++i)
    {
      auto const value {RandomUInt{}()};

      auto collect {NoThrowResultsSumOfSuccesses{}};

      multi_client::call_and_collect<Ping>
        ( multi_client::command_generator::Const<Ping> {value}
        , collect
        , clients
        , multi_client::ParallelCalls::Unlimited{}
        );

      ASSERT_EQ
        ( collect._sum
        , std::optional<unsigned int> {value * servers.size()}
        );
    }
  }

  TYPED_TEST
    ( RPCMultiClientT
    , access_policy_Concurrent_works_with_nothrow_collectors_and_concurrent_calls
    )
  {
    // \note The same test with a _throwing_ collector fails because
    // it would drop async operations, which is only allowed for the
    // access policy Sequential. See drop_async_operation, too.

    using Protocol = TypeParam;

    auto servers {std::list<Server<Protocol>> {RandomUInt {20u, 100u}()}};
    auto clients
      {this->template make_clients<access_policy::Concurrent> (servers)};

    auto const number_of_parallel_calls {RandomUInt {2u, 100u}()};

    auto collectors {std::list<NoThrowResultsSumOfSuccesses>{}};
    auto calls {std::list<std::future<void>>{}};
    auto const value {RandomUInt{}()};

    for (auto i {0u}; i < number_of_parallel_calls; ++i)
    {
      auto& collector {collectors.emplace_back()};

      calls.emplace_back
        ( std::async
          ( std::launch::async
          , [&]
            {
              multi_client::call_and_collect<Ping>
                ( multi_client::command_generator::Const<Ping> {value}
                , collector
                , clients
                , multi_client::ParallelCalls::Unlimited{}
                );
            }
          )
        );
    }

    // \todo ensure the calls are running in parallel

    for (auto& call : calls)
    {
      call.get();
    }
    for (auto const& collector : collectors)
    {
      ASSERT_EQ
        ( collector._sum
        , std::optional<unsigned int> {value * servers.size()}
        );
    }
  }

  // \todo test multi_call using clients of different type
}
