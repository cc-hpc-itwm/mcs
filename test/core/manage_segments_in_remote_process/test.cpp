// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/one_of.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/read/read.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
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
    template<class> struct MCSCoreManageSegmentsInRemoteProcessT
      : public testing::random::Test{}
      ;
    TYPED_TEST_SUITE ( MCSCoreManageSegmentsInRemoteProcessT
                     , ProtocolsAndStorages
                     );
  }

  // Publish the ID of the storage provider and let remote processes
  // create and remove segments to storages. Verify the segments have
  // been created/removed by accessing size_used of its storage.
  //
  TYPED_TEST ( MCSCoreManageSegmentsInRemoteProcessT
             , remote_process_can_manage_segments_of_storages
             )
  {
    using Protocol = typename TypeParam::Protocol;
    using TestingStorage = typename TypeParam::Storage;

    // create a couple a storages
    auto const testing_storages
      { std::invoke
        ( [&]
          {
            auto _testing_storages {std::list<TestingStorage>{}};

            auto random_capacity
              {testing::random::value<std::size_t> {10 << 20, 20 << 20}};

            auto N {testing::random::value<int> {1, 10}()};

            while (N --> 0)
            {
              _testing_storages.emplace_back
                ( fmt::format ("{}", N)
                , storage::MaxSize
                    { storage::MaxSize::Limit
                        {memory::make_size (random_capacity())}
                    }
                );
            }

            return _testing_storages;
          }
        )
      };

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    auto unique_storages
      { std::vector
        < SupportedStorageImplementations::template wrap
            < core::UniqueStorage
            , typename TestingStorage::Storage
            >
        >{}
      };

    auto const storage_ids
      { std::invoke
        ( [&]
          {
            auto _storage_ids
              {std::unordered_map< storage::ID
                                 , std::reference_wrapper<TestingStorage const>
                                 >{}
              };

            for (auto const& testing_storage : testing_storages)
            {
              _storage_ids.emplace
                ( unique_storages.emplace_back
                    ( make_unique_storage<typename TestingStorage::Storage>
                      ( std::addressof (storages)
                      , testing_storage.parameter_create()
                      )
                    )->id()
                , testing_storage
                );
            }

            return _storage_ids;
          }
        )
      };

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

    auto const temporary_directory
      { testing::UniqTemporaryDirectory
          {"MCS_TEST_CORE_TEST_MANAGE_SEGMENTS_IN_REMOTE_PROCESS_TEST"}
      };
    auto const segment_id_file {temporary_directory.path() / "segment_id"};

    auto segment_ids
      {std::unordered_map< storage::ID
                         , std::unordered_set<storage::segment::ID>
                         >{}
      };

    for (auto const& [storage_id, testing_storage] : storage_ids)
    {
      auto random_segment_size
        {testing::random::value<std::size_t> {2 << 20, 5 << 20}};

      auto const max_size
        { storages.template size_max<typename TestingStorage::Storage>
            ( storages.read_access()
            , storage_id
            , testing_storage.get().parameter_size_max()
            )
        };

      for ( auto size_used {memory::make_size (0)}
          , size {memory::make_size (random_segment_size())}

          ; ! (size_used + size > max_size)

          ; size_used += size
          , size = memory::make_size (random_segment_size())
          )
      {
        ASSERT_EQ
          ( EXIT_SUCCESS
          , std::system
            ( fmt::format
              ( "{} '{}' '{}' '{}' '{}' '{}'"
              , MCS_TEST_CORE_TEST_MANAGE_SEGMENTS_IN_REMOTE_PROCESS_SEGMENT_CREATE
              , util::ASIO::make_connectable (provider.local_endpoint())
              , storage_id
              , storage::make_parameter
                  (testing_storage.get().parameter_segment_create())
              , size
              , segment_id_file
              ).c_str()
            )
          );

        ASSERT_TRUE
          ( segment_ids[storage_id].emplace
              ( util::read::from_file<storage::segment::ID> (segment_id_file)
              ).second
          );

        ASSERT_EQ
          ( size_used + size
          , storages.template size_used<typename TestingStorage::Storage>
              ( storages.read_access()
              , storage_id
              , testing_storage.get().parameter_size_used()
              )
          );
      }
    }

    // Remove all segments and assert size_used is back to zero once
    // all segments have been removed from a certain storage.
    //
    while (!segment_ids.empty())
    {
      auto& [storage_id, ids] {testing::random::one_of (segment_ids)};
      auto const& testing_storage {storage_ids.at (storage_id)};
      auto const segment_id {testing::random::one_of (ids)};

      ASSERT_EQ
        ( EXIT_SUCCESS
        , std::system
          ( fmt::format
            ( "{} '{}' '{}' '{}' '{}'"
            , MCS_TEST_CORE_TEST_MANAGE_SEGMENTS_IN_REMOTE_PROCESS_SEGMENT_REMOVE
            , util::ASIO::make_connectable (provider.local_endpoint())
            , storage_id
            , storage::make_parameter
                (testing_storage.get().parameter_segment_remove())
            , segment_id
            ).c_str()
          )
        );

      ids.erase (segment_id);

      if (ids.empty())
      {
        ASSERT_EQ
          ( memory::make_size (0)
          , storages.template size_used<typename TestingStorage::Storage>
              ( storages.read_access()
              , storage_id
              , testing_storage.get().parameter_size_used()
              )
          );

        segment_ids.erase (storage_id);
      }
    }
  }
}
