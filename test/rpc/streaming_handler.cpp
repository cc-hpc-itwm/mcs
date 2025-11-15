// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/read.hpp>
#include <functional>
#include <gtest/gtest.h>
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
#include <variant>
#include <vector>

namespace
{
  using Bytes = std::vector<int>;

  struct Copy
  {
    Bytes xs;

    using Response = bool;
  };

  struct Stream
  {
    Stream (Bytes bytes) : bytes_or_size {bytes} {}
    Stream (std::size_t size) : bytes_or_size {size} {}

    // Bytes on serialize, std::size_t on deserialize
    std::variant<Bytes, std::size_t> bytes_or_size;

    using Response = bool;
  };
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (Copy);
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION (Stream);
}

namespace mcs::serialization
{
  // Difference on serialize: append versus stream
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT (oa, x, Copy)
  {
    save (oa, x.xs.size());
    oa.append (std::span {x.xs});

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT (oa, x, Stream)
  {
    // Bytes on serialize
    auto const& xs {std::get<Bytes> (x.bytes_or_size)};
    save (oa, xs.size());
    oa.stream (std::span {xs});

    return oa;
  }

  // Difference on deserialize: Stream does not restore the bytes,
  // instead the handler streams the data from the socket.
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT (ia, Copy)
  {
    auto const size {load<std::size_t> (ia)};
    auto xs {Bytes{}};
    xs.resize (size);
    ia.extract (xs.data(), size * sizeof (int));
    return Copy {std::move (xs)};
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT (ia, Stream)
  {
    auto const size {load<std::size_t> (ia)};
    return Stream {size};
  }
}

namespace
{
  static_assert (mcs::rpc::is_command<Copy>);
  static_assert (mcs::rpc::is_command<Stream>);

  struct Handler
  {
    Handler (Bytes const& expected)
      : _expected {expected}
    {}

    auto operator() (Copy copy) const -> bool
    {
      // The command includes the bytes.
      return copy.xs == _expected;
    }

    // The handler tells the Dispatcher that it wants to stream data
    // simply by accepting the socket as additional parameter.
    template<typename Socket>
      auto operator() (Stream stream, Socket& socket) const -> bool
    {
      // The command includes the size only, the bytes can be streamed
      // from the socket into any location. The location may depend on
      // the meta data included in the command.
      auto const size {std::get<std::size_t> (stream.bytes_or_size)};
      auto xs {std::vector<int>{}};
      xs.resize (size);
      auto const num_bytes {size * sizeof (int)};
      auto const bytes_read
        {asio::read (socket, asio::buffer (xs.data(), num_bytes))};
      return bytes_read == num_bytes && xs == _expected;
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
  TYPED_TEST (RPCStream, handler_can_stream_data)
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
    using Dispatcher = rpc::Dispatcher<Handler, Copy, Stream>;

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

    EXPECT_TRUE (client (Copy {xs}));
    EXPECT_TRUE (client (Stream {xs}));
  }
}
