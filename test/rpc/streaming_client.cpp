// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <fmt/format.h>
#include <functional>
#include <gtest/gtest.h>
#include <mcs/Error.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Dispatcher.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/buffer/Bytes.hpp>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

namespace
{
  using Bytes = std::vector<int>;

  struct Put
  {
    Put (Bytes bytes) : bytes_or_size {bytes} {}
    Put (std::size_t size) : bytes_or_size {size} {}

    // Bytes on serialize, size on deseralize
    std::variant<Bytes, std::size_t> bytes_or_size;

    template<typename Socket>
      auto stream (Socket& socket) const -> void
    {
      auto const& xs {std::get<Bytes> (bytes_or_size)};
      auto const bytes_written
        { asio::write ( socket
                      , asio::buffer (xs.data(), xs.size() * sizeof (int))
                      )
        };

      if (bytes_written != xs.size() * sizeof (int))
      {
        throw std::runtime_error
          { fmt::format ( "Could not write all data: {} != {}"
                        , bytes_written
                        , xs.size() * sizeof (int)
                        )
          };
      }
    }

    using Response = bool;
  };

  struct Get
  {
    Get ( std::size_t size_
        )
      : size {size_}
    {}
    Get ( std::size_t size_
        , std::reference_wrapper<std::vector<int>> sink_
        )
      : size {size_}
      , sink {sink_}
    {}

    std::size_t size;
    std::optional<std::reference_wrapper<std::vector<int>>> sink{};

    template<typename Socket>
      auto stream (Socket& socket) const -> void
    {
      if (!sink)
      {
        throw mcs::Error {"Get::stream: no sink"};
      }

      auto& xs {sink->get()};
      xs.resize (size);
      auto const bytes_read
        { asio::read ( socket
                     , asio::buffer (xs.data(), xs.size() * sizeof (int))
                     )
        };

      if (bytes_read != xs.size() * sizeof (int))
      {
        throw std::runtime_error
          { fmt::format ( "Could not read all data: {} != {}"
                        , bytes_read
                        , xs.size() * sizeof (int)
                        )
          };
      }
    }

    using Response = bool;
  };
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (Put);
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (Get);
}

namespace mcs::serialization
{
  // Serialize size only, the stream operator will write the data
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT (oa, x, Put)
  {
    auto const& xs {std::get<Bytes> (x.bytes_or_size)};
    save (oa, xs.size());

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT (ia, Put)
  {
    auto const size {load<std::size_t> (ia)};

    return Put {size};
  }

  // Serialize size only, the stream operator will read the data
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT (oa, x, Get)
  {
    save (oa, x.size);

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT (ia, Get)
  {
    auto const size {load<std::size_t> (ia)};

    return Get {size};
  }
}

namespace
{
  static_assert (mcs::rpc::is_command<Put>);
  static_assert (mcs::rpc::is_command<Get>);

  struct Handler
  {
    Handler (Bytes const& expected)
      : _expected {expected}
    {}

    template<typename Socket>
      auto operator() (Put put, Socket& socket) const -> bool
    {
      auto const size {std::get<std::size_t> (put.bytes_or_size)};
      auto xs {std::vector<int>{}};
      xs.resize (size);
      auto const num_bytes {size * sizeof (int)};
      auto const bytes_read
        {asio::read (socket, asio::buffer (xs.data(), num_bytes))};
      return bytes_read == num_bytes && xs == _expected;
    }

    template<typename Socket>
      auto operator() (Get get, Socket& socket) const -> bool
    {
      auto const num_bytes {get.size * sizeof (int)};
      auto const bytes_written
        {asio::write (socket, asio::buffer (_expected.data(), num_bytes))};
      return bytes_written == num_bytes;
    }

  private:
    Bytes const& _expected;
  };

  using Protocols = ::testing::Types
      < asio::ip::tcp
      , asio::local::stream_protocol
      >;
  template<class> struct RPCStream : public mcs::testing::random::Test{};
  TYPED_TEST_SUITE (RPCStream, Protocols);
}

namespace mcs::rpc
{
  TYPED_TEST (RPCStream, client_can_stream_data)
  {
    auto const xs
      { std::invoke
        ( []
          {
            auto const size
              {testing::random::value<std::size_t> {0, 4 << 10}()};
            auto xs_ {std::vector<int>{}};
            xs_.resize (size);
            std::ranges::generate
              ( xs_
              , [random_int = testing::random::value<int>{}]() mutable
                {
                  return random_int();
                }
              );
            return xs_;
          }
        )
      };

    using Protocol = TypeParam;
    using Dispatcher = rpc::Dispatcher<Handler, Put, Get>;

    auto io_context_provider
      { ScopedRunningIOContext
          { ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };
    auto const protocol_state {testing::RPC::ProtocolState<Protocol>{}};
    auto const provider
      { rpc::make_provider<Protocol, Dispatcher>
          ( protocol_state.local_endpoint()
          , io_context_provider
          , xs
          )
      };

    auto io_context_client
      { ScopedRunningIOContext
          { ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };
    auto const client
      { rpc::make_client<Protocol, Dispatcher, access_policy::Exclusive>
          ( io_context_client
          , provider.local_endpoint()
          )
      };

    EXPECT_TRUE (client (Put {xs}));

    {
      std::vector<int> sink;
      EXPECT_TRUE (client (Get {xs.size(), sink}));
      EXPECT_EQ (xs, sink);
    }
  }
}
