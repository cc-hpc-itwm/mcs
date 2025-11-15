// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <fmt/format.h>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
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

    namespace Impl = testing::core::storage::implementation;

    using ProtocolsAndStorages = ::testing::Types
      < ProtocolAndStorage<asio::ip::tcp               , Impl::Files>
      , ProtocolAndStorage<asio::ip::tcp               , Impl::SHMEM>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::Files>
      , ProtocolAndStorage<asio::local::stream_protocol, Impl::SHMEM>
      >;
    template<class> struct MCSShareBetweenClientsT
      : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSShareBetweenClientsT, ProtocolsAndStorages);
  }

  // Create some random ints, call the reverse_int process and expect
  // the data to be reversed without any memory_copy neither here nor
  // in the reverse_ints process. Note that the reverse_int process
  // does not have knowledge about the storage implementation.
  //
  TYPED_TEST ( MCSShareBetweenClientsT
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

    // create a storage provider for remote access
    auto io_context_provider
      { rpc::ScopedRunningIOContext
          {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
      };
    auto const protocol_state {testing::RPC::ProtocolState<Protocol>{}};
    auto const provider
      { control::Provider<Protocol, SupportedStorageImplementations>
          { io_context_provider
          , protocol_state.local_endpoint()
          , std::addressof (storages)
          }
      };

    // call external process to reverse the data...
    ASSERT_EQ
      ( EXIT_SUCCESS
      , std::system
        ( fmt::format
          ( "{} '{}' '{}' '{}' '{}' '{}'"
          , MCS_CORE_TEST_SHARE_BETWEEN_PROCESSES_REVERSE_INTS
          , util::ASIO::make_connectable (provider.local_endpoint())
          , storage->id()
          , segment->id()
          , number_of_bytes
          , storage::make_parameter (parameter_chunk_description)
          ).c_str()
        )
      );

    // ...and expect the local data to be reversed
    std::ranges::reverse (elements);
    ASSERT_THAT (elements, ::testing::ElementsAreArray (ints));
  }

  namespace
  {
    using Protocols = ::testing::Types
      < asio::ip::tcp
      , asio::local::stream_protocol
      >;
    template<class> struct MCSShareBetweenClientsPersistentT
      : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSShareBetweenClientsPersistentT, Protocols);
  }

  TYPED_TEST ( MCSShareBetweenClientsPersistentT
             , produce_in_one_and_share_in_another_producer
             )
  {
    using Protocol = TypeParam;
    using TestingStorage = testing::core::storage::implementation::Files;

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

    auto testing_storage {TestingStorage{}};

    using ParameterSegment =
      typename TestingStorage::Storage::Parameter::Segment
      ;

    // Create a persistent segment in a storage and destroy the Storages
    //
    auto const segment_id
      { std::invoke
        ( [&]
          {
            auto storages
              { core::Storages<SupportedStorageImplementations>{}
              };
            auto const storage
              { make_unique_storage<typename TestingStorage::Storage>
                  ( std::addressof (storages)
                  , testing_storage.parameter_create()
                  )
              };
            return storage::make_unique_segment
                < typename TestingStorage::Storage
                >
              ( std::addressof (storages)
              , storage->id()
              , number_of_bytes
              , testing_storage.parameter_segment_create
                  ( typename ParameterSegment::OnRemove::Keep{}
                  )
              , testing_storage.parameter_segment_remove()
              )->id()
              ;
          }
        )
      };

    // create new Storages and create the same storage, the persistent
    // segment is recovered by the Files-storage and can be used for
    // sharing
    //
    auto storages {core::Storages<SupportedStorageImplementations>{}};
    auto const storage
      { make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (storages)
          , testing_storage.parameter_create()
          )
      };

    // Even though no segment has been created using the new
    // controller, the persistent segment is available and occupies
    // space
    //
    ASSERT_EQ
      ( number_of_bytes
      , storages.template size_used<typename TestingStorage::Storage>
          ( storages.read_access()
          , storage->id()
          , testing_storage.parameter_size_used()
          )
      );

    auto const parameter_chunk_description
      { testing_storage.parameter_chunk_description()
      };
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
          , segment_id
          , memory::make_range (memory::make_offset (0), number_of_bytes)
          )
        }
      };
    auto const ints {as<int> (chunk)};
    std::ranges::copy (elements, std::begin (ints));

    // create a storage provider for remote access
    auto io_context_provider
      { rpc::ScopedRunningIOContext
          {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
      };
    auto const protocol_state {testing::RPC::ProtocolState<Protocol>{}};
    auto const provider
      { control::Provider<Protocol, SupportedStorageImplementations>
          { io_context_provider
          , protocol_state.local_endpoint()
          , std::addressof (storages)
          }
      };

    // call external process to reverse the data...
    ASSERT_EQ
      ( EXIT_SUCCESS
      , std::system
        ( fmt::format
          ( "{} '{}' '{}' '{}' '{}' '{}'"
          , MCS_CORE_TEST_SHARE_BETWEEN_PROCESSES_REVERSE_INTS
          , util::ASIO::make_connectable (provider.local_endpoint())
          , storage->id()
          , segment_id
          , number_of_bytes
          , storage::make_parameter (parameter_chunk_description)
          ).c_str()
        )
      );

    // ...and expect the local data to be reversed
    std::ranges::reverse (elements);
    ASSERT_THAT (elements, ::testing::ElementsAreArray (ints));

    // The segment can be removed using the ForceRemoval flag
    //
    ASSERT_EQ
      ( number_of_bytes
      , storages.template segment_remove<typename TestingStorage::Storage>
        ( storages.write_access()
        , storage->id()
        , testing_storage.parameter_segment_remove
            ( typename ParameterSegment::ForceRemoval{}
            )
        , segment_id
        )
      );

    ASSERT_EQ
      ( memory::make_size (0)
      , storages.template size_used<typename TestingStorage::Storage>
          ( storages.read_access()
          , storage->id()
          , testing_storage.parameter_size_used()
          )
      );
  }
}
