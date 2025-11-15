// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstdlib>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/core/random/memory/Size.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <ranges>
#include <vector>

namespace mcs::core
{
  namespace
  {
    template<typename P, typename S>
      struct ProtocolAndStorage
    {
      using Protocol = P;
      using Storage = S;
    };

    using SupportedStorageImplementations = util::type::List
      < core::storage::implementation::Files
      , core::storage::implementation::Heap
      , core::storage::implementation::SHMEM
      , core::storage::implementation::Virtual
      >;

    namespace Impl = testing::core::storage::implementation;

    using ProtocolsAndStorages = ::testing::Types
      < ProtocolAndStorage<asio::ip::tcp               , Impl::Files>
      , ProtocolAndStorage<asio::ip::tcp               , Impl::Heap>
      , ProtocolAndStorage<asio::ip::tcp               , Impl::SHMEM>
      , ProtocolAndStorage<asio::ip::tcp               , Impl::Virtual<Impl::Files>>
      , ProtocolAndStorage<asio::ip::tcp               , Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorage<asio::ip::tcp               , Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::Files>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::Heap>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::SHMEM>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>>
      >;
    template<class> struct MCSShareInTheSameProcessT
      : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSShareInTheSameProcessT, ProtocolsAndStorages);
  }

  TYPED_TEST ( MCSShareInTheSameProcessT
             , produce_call_reverse_and_expect_reversed_works
             )
  {
    using Protocol = typename TypeParam::Protocol;
    using TestingStorage = typename TypeParam::Storage;

    // create some random elements
    using RandomSize = testing::random::value<std::size_t>;
    using RandomElement = testing::random::value<int>;

    auto const number_of_ints {RandomSize {RandomSize::Max {1 << 20}}()};
    auto const number_of_bytes
      {memory::make_size (number_of_ints * sizeof (int))};
    auto elements {std::vector<int>{}};
    std::generate_n ( std::back_inserter (elements)
                    , number_of_ints
                    , RandomElement{}
                    );

    // create a testing storage and fill the random elements into a segment
    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto testing_storage {TestingStorage{}};
    auto const storage
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )
      };
    auto const segment
      { storage::make_unique_segment<typename TestingStorage::Storage>
        ( std::addressof (storages)
        , storage->id()
        , number_of_bytes
        , testing_storage.parameter_segment_create()
        , testing_storage.parameter_segment_remove()
        )
      };

    auto const parameter_chunk_description
      {testing_storage.parameter_chunk_description()};
    {
      auto const chunk
        { SupportedStorageImplementations::template wrap
            < Chunk
            , chunk::access::Mutable
            >
          { storages.template chunk_description
              < typename TestingStorage::Storage
              , chunk::access::Mutable
              >
            ( storages.read_access()
            , storage->id()
            , parameter_chunk_description
            , segment->id()
            , memory::make_range (memory::make_offset (0), number_of_bytes)
            )
          }
        };
      auto const ints {as<int> (chunk)};
      std::ranges::copy (elements, std::begin (ints));
    }

    // reverse the local elements
    std::ranges::reverse (elements);

    // create a storage provider for remote access
    auto io_context_provider
      { rpc::ScopedRunningIOContext
          {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
      };
    auto const provider
      { control::Provider<Protocol, SupportedStorageImplementations>
          { io_context_provider
          , typename Protocol::endpoint{}
          , std::addressof (storages)
          }
      };

    // create a client for that provider...
    auto io_context_client
      { rpc::ScopedRunningIOContext
          {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
      };
    auto const client
      { control::Client
            < Protocol
            , rpc::access_policy::Exclusive
            , SupportedStorageImplementations
            >
          { io_context_client
          , util::ASIO::make_connectable (provider.local_endpoint())
          }
      };

    auto const chunk
      { SupportedStorageImplementations::template wrap
            < Chunk
            , chunk::access::Mutable
            >
        { client.template chunk_description<chunk::access::Mutable>
          ( storage->id()
          , storage::make_parameter (parameter_chunk_description)
          , segment->id()
          , memory::make_range (memory::make_offset (0), number_of_bytes)
          )
        }
      };

    auto const ints {as<int> (chunk)};
    std::ranges::reverse (ints);

    ASSERT_THAT (elements, ::testing::ElementsAreArray (ints));
  }
}
