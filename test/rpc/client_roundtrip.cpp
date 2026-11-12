// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/Timer.hpp>
#include <mcs/util/timed.hpp>
#include <mcs/util/timer/Ticks.hpp>
#include <utility>
#include <vector>

namespace
{
  struct Handler
  {
    struct Echo
    {
      using Response = int;

      int value;
    };

    auto operator() (Echo request) const noexcept -> int
    {
      return request.value;
    }
  };

  template<typename Protocol>
    auto measure (unsigned int workers) -> void
  {
    using Dispatcher = mcs::rpc::Dispatcher<Handler, Handler::Echo>;

    auto provider_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        }
      };
    auto client_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {workers}
        }
      };

    auto const provider
      { mcs::rpc::make_provider<Protocol, Dispatcher>
        ( {}
        , provider_context
        )
      };
    auto const client
      { mcs::rpc::make_client< Protocol
                             , Dispatcher
                             , mcs::rpc::access_policy::Exclusive
                             >
        ( client_context
        , provider.local_endpoint()
        )
      };

    auto const value {std::invoke (mcs::testing::random::value<int>{})};

    for (auto i {0}; i < 1000; ++i)
    {
      ASSERT_EQ (client (Handler::Echo {value}), value);
    }

    static constexpr auto clock_scale {1'000'000L};

    using Ticks = mcs::util::timer::Ticks<clock_scale>;

    auto samples {std::vector<Ticks>{}};
    samples.reserve (10000);

    auto timer {mcs::util::Timer<clock_scale>{}};

    for (auto i {0}; i < 10000; ++i)
    {
      auto const [response, time]
        { mcs::util::timed<clock_scale>
          ( [&]
            {
              return client (Handler::Echo {value});
            }
          )
        };

      ASSERT_EQ (response, value);

      samples.emplace_back (time);
    }

    auto const time {std::move (timer).stop()};

    std::ranges::sort (samples, {}, &Ticks::value);

    fmt::print
      ( "workers {} median {} p95 {} calls_per_second {}\n"
      , workers
      , samples[5000]
      , samples[9500]
      , time.per_second (samples.size())
      );
  }

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;

  template<typename>
    struct ClientRoundtrip : public mcs::testing::random::Test{};

  TYPED_TEST_SUITE (ClientRoundtrip, Protocols);
}

TYPED_TEST (ClientRoundtrip, one_worker)
{
  measure<TypeParam> (1u);
}

TYPED_TEST (ClientRoundtrip, four_workers)
{
  measure<TypeParam> (4u);
}
