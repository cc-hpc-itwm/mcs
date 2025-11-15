// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <chrono>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <mcs/block_device/Block.hpp>
#include <mcs/block_device/Buffer.hpp>
#include <mcs/block_device/Reader.hpp>
#include <mcs/block_device/Storage.hpp>
#include <mcs/block_device/Writer.hpp>
#include <mcs/block_device/block/Count.hpp>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/block_device/block/Range.hpp>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/block_device/meta_data/Client.hpp>
#include <mcs/block_device/meta_data/Provider.hpp>
#include <mcs/block_device/storage/with_range.hpp>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/core/transport/implementation/ASIO/Client.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/member_AUTO.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::Heap
    , mcs::core::storage::implementation::SHMEM
    >;

  template<mcs::util::ASIO::is_protocol Protocol>
    using TransportProvider
     = mcs::core::transport::implementation::ASIO::Provider
        < Protocol
        , SupportedStorageImplementations
        >
    ;

  template< typename Element
          , mcs::rpc::is_protocol Protocol
          , typename TestingStorage
          >
    struct StoragesProvider
  {
    template<typename Tag>
      StoragesProvider
        ( mcs::core::memory::Size size
        , Tag tag
        , Element zero
        )
          : _testing_storage {fmt::format ("P-{}", tag)}
          , _size {size}
          , _protocol_state {fmt::format ("P-{}", tag)}
    {
      auto const chunk
        { SupportedStorageImplementations::template wrap
             < mcs::core::Chunk
             , mcs::core::chunk::access::Mutable
             >
           { _used_storages
           . template chunk_description
               < typename TestingStorage::Storage
               , mcs::core::chunk::access::Mutable
               >
             ( _used_storages.read_access()
             , _storage->id()
             , _testing_storage.parameter_chunk_description()
             , _segment->id()
             , mcs::core::memory::make_range
                 (mcs::core::memory::make_offset (0), size)
             )
           }
        };
      auto const elements {mcs::core::as<Element> (chunk)};

      std::iota (std::begin (elements), std::end (elements), zero);
    }

    [[nodiscard]] auto storage() const -> mcs::block_device::Storage
    {
      return _block_device_storage;
    }

  private:
    mcs::core::Storages<SupportedStorageImplementations> _used_storages{};
    TestingStorage _testing_storage;
    mcs::core::memory::Size _size;

    MCS_UTIL_MEMBER_AUTO
      ( _storage
      , mcs::core::make_unique_storage<typename TestingStorage::Storage>
          ( std::addressof (_used_storages)
          , _testing_storage.parameter_create()
          )
      );
    static_assert
      ( std::is_same_v
        < decltype (_storage)
        , SupportedStorageImplementations::template wrap
            < mcs::core::UniqueStorage
            , typename TestingStorage::Storage
            >
        >
      );

    SupportedStorageImplementations::template wrap
      < mcs::core::storage::UniqueSegment
      , typename TestingStorage::Storage
      > _segment
        { mcs::core::storage::make_unique_segment
              < typename TestingStorage::Storage
              >
            ( std::addressof (_used_storages)
            , _storage->id()
            , _size
            , _testing_storage.parameter_segment_create()
            , _testing_storage.parameter_segment_remove()
            )
        };
    mcs::rpc::ScopedRunningIOContext _io_context
      {mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM};
    mcs::testing::RPC::ProtocolState<Protocol> _protocol_state;
    TransportProvider<Protocol> _transport_provider
      { _io_context
      , _protocol_state.local_endpoint()
      , std::addressof (_used_storages)
      };
    mcs::block_device::Storage _block_device_storage
      { _transport_provider.connection_information()
      , _storage->id()
      , mcs::core::storage::make_parameter
          (_testing_storage.parameter_chunk_description())
      , _segment->id()
      , mcs::core::memory::make_range
          (mcs::core::memory::make_offset (0), _size)
      };
  };

  using Protocols = ::testing::Types
    < asio::ip::tcp
    , asio::local::stream_protocol
    >;
  template<class> struct MCSBlockDeviceT : public ::testing::Test{};
  TYPED_TEST_SUITE (MCSBlockDeviceT, Protocols);
}

namespace mcs::block_device
{
  TYPED_TEST (MCSBlockDeviceT, basics)
  {
    auto const storage_size {32 << 20};
    using Element = long;
    auto const elements_per_storage {storage_size / sizeof (Element)};
    auto const provider_tcp_Files
      { StoragesProvider< Element
                        , asio::ip::tcp
                        , testing::core::storage::implementation::Files
                        >
        { core::memory::make_size (storage_size)
        , "tF"
        , 0 * elements_per_storage
        }
      };
    auto const provider_tcp_Heap
      { StoragesProvider< Element
                        , asio::ip::tcp
                        , testing::core::storage::implementation::Heap
                        >
        { core::memory::make_size (storage_size)
        , "tH"
        , 1 * elements_per_storage
        }
      };
    auto const provider_tcp_SHMEM
      { StoragesProvider< Element
                        , asio::ip::tcp
                        , testing::core::storage::implementation::SHMEM
                        >
        { core::memory::make_size (storage_size)
        , "tS"
        , 2 * elements_per_storage
        }
      };

    auto const provider_local_stream_Files
      { StoragesProvider< Element
                        , asio::local::stream_protocol
                        , testing::core::storage::implementation::Files
                        >
        { core::memory::make_size (storage_size)
        , "sF"
        , 3 * elements_per_storage
        }
      };
    auto const provider_local_stream_Heap
      { StoragesProvider< Element
                        , asio::local::stream_protocol
                        , testing::core::storage::implementation::Heap
                        >
        { core::memory::make_size (storage_size)
        , "sH"
        , 4 * elements_per_storage
        }
      };
    auto const provider_local_stream_SHMEM
      { StoragesProvider< Element
                        , asio::local::stream_protocol
                        , testing::core::storage::implementation::SHMEM
                        >
        { core::memory::make_size (storage_size)
        , "sS"
        , 5 * elements_per_storage
        }
      };

    auto const bs {4 << 10};
    auto const block_size {block::make_size (bs)};
    auto blocks {meta_data::Blocks {block_size}};

    auto const check_add
      { [&] (auto const& provider, auto begin, auto end, auto count)
        {
          auto const [range, unused] {blocks.add (provider.storage())};

          ASSERT_TRUE (range.has_value());
          ASSERT_FALSE (unused.has_value());
          ASSERT_EQ (blocks.number_of_blocks(), block::make_count (count));
          ASSERT_EQ (block::begin (range.value()), block::make_id (begin));
          ASSERT_EQ (block::end (range.value()), block::make_id (end));
        }
      };

    // add some storages
    check_add (provider_tcp_Files         , 0*8192, 1*8192, 1*8192);
    check_add (provider_tcp_Heap          , 1*8192, 2*8192, 2*8192);
    check_add (provider_tcp_SHMEM         , 2*8192, 3*8192, 3*8192);
    check_add (provider_local_stream_Files, 3*8192, 4*8192, 4*8192);
    check_add (provider_local_stream_Heap , 4*8192, 5*8192, 5*8192);
    check_add (provider_local_stream_SHMEM, 5*8192, 6*8192, 6*8192);

    // 0      1      2      3      4      5      6
    // 0 tf   8 tH   16 tS  24 lF  32 lH  40 lS  48
    // [-----)[-----)[-----)[-----)[-----)[-----)

    ASSERT_EQ
      ( blocks.blocks()
      , std::list<block::Range>
          {block::make_range (block::make_id (0), block::make_id (6*8192))}
      );

    auto const check_remove
      { [&] (auto begin, auto count, std::list<Storage> unused)
        {
          auto const result {blocks.remove (block::make_range (begin, count))};

          ASSERT_EQ (result.unused, unused);
        }
      };

    // remove a complete storage
    check_remove
      ( block::make_id (2*8192), block::make_count (8192)
      , {provider_tcp_SHMEM.storage()}
      );

    // 0      1             3      4      5      6
    // 0 tf   8 tH          24 lF  32 lH  40 lS  48
    // [-----)[-----)       [-----)[-----)[-----)

    ASSERT_EQ
      ( blocks.blocks()
      , ( std::list<block::Range>
          { block::make_range (block::make_id (0), block::make_id (2*8192))
          , block::make_range (block::make_id (3*8192), block::make_id (6*8192))
          }
        )
      );

    // remove part of a storage
    check_remove
      ( block::make_id (11000), block::make_count (100)
      , { storage::with_range
            ( provider_tcp_Heap.storage()
            , core::memory::make_offset ((11000-8192+  0) * bs)
            , core::memory::make_offset ((11000-8192+100) * bs)
            )
         }
      );

    // 0      1             3      4      5      6
    // 0 tf   8 tH          24 lF  32 lH  40 lS  48
    // [-----)[-) [-)       [-----)[-----)[-----)
    //            ^11100
    //          ^11000

    ASSERT_EQ
      (blocks.number_of_blocks(), block::make_count (4*8192 + (8192 - 100)));

    ASSERT_EQ
      ( blocks.blocks()
      , ( std::list<block::Range>
          { block::make_range (block::make_id (0), block::make_id (11000))
          , block::make_range (block::make_id (11100), block::make_id (2*8192))
          , block::make_range (block::make_id (3*8192), block::make_id (6*8192))
          }
        )
      );

    auto const check_block
      { [&] (auto block_id, auto const& provider, auto shift_offset)
        {
          auto [connectable, address]
            {blocks.location (block::make_id (block_id))};
          auto [storage_id, storage_parameter, segment_id, offset] {address};
          auto s {provider.storage()};

          ASSERT_EQ (connectable, s._provider_connectable);
          ASSERT_EQ (storage_id, s._storage_id);
          ASSERT_EQ (storage_parameter, s._storage_parameter_chunk_description);
          ASSERT_EQ (segment_id, s._segment_id);
          ASSERT_EQ (offset, begin (s._range) + block::make_count (shift_offset) * block_size);
        }
      };

    check_block (0, provider_tcp_Files, 0);
    check_block (1, provider_tcp_Files, 1);
    check_block (8191, provider_tcp_Files, 8191);

    check_block (8192, provider_tcp_Heap, 0);
    check_block (10000, provider_tcp_Heap, 1808);
    check_block (16383, provider_tcp_Heap, 8191);

    auto const location_misses_block
      { [&] (auto block_id)
        {
          testing::require_exception
            ( [&]
              {
                std::ignore = blocks.location (block_id);
              }
            , testing::Assert<meta_data::Blocks::Error::BlockNotInAnyStorage>
              { [&] (auto const& caught)
                {
                  ASSERT_EQ (caught.block_id(), block_id);
                  ASSERT_STREQ
                    ( caught.what()
                    , fmt::format
                      ( "mcs::block_device::meta_data::Blocks::BlockNotInAnyStorage: {}\n"
                      , block_id
                      ).c_str()
                    );
                }
              }
            );
        }
      };

    location_misses_block (block::make_id (16384));
    location_misses_block (block::make_id (16385));
    location_misses_block (block::make_id (24575));

    check_block (24576, provider_local_stream_Files, 0);

    check_block (49151, provider_local_stream_SHMEM, 8191);

    location_misses_block (block::make_id (49152));
    location_misses_block (block::make_id (50000));

    check_add (provider_tcp_SHMEM, 6*8192, 7*8192, 5*8192 + (8192 - 100));

    // 0      1             3      4      5      6      7
    // 0 tf   8 tH          24 lF  32 lH  40 lS  48 tS  56
    // [-----)[-) [-)       [-----)[-----)[-----)[-----)
    //            ^11100
    //          ^11000

    ASSERT_EQ
      ( blocks.blocks()
      , ( std::list<block::Range>
          { block::make_range (block::make_id (0), block::make_id (11000))
          , block::make_range (block::make_id (11100), block::make_id (2*8192))
          , block::make_range (block::make_id (3*8192), block::make_id (7*8192))
          }
        )
      );

    check_block (50000, provider_tcp_SHMEM, 848);

    // remove blocks that span over multiple storages
    check_remove
      ( block::make_id (2000), block::make_count (45000)
      , { storage::with_range
            ( provider_tcp_Files.storage()
            , core::memory::make_offset (bs * 2000)
            , core::memory::make_offset (bs * 8192)
            )
        , storage::with_range
           ( provider_tcp_Heap.storage()
           , core::memory::make_offset (bs *               0 )
           , core::memory::make_offset (bs * (11000-8192+  0))
           )
        , storage::with_range
           ( provider_tcp_Heap.storage()
           , core::memory::make_offset (bs * (11000-8192+100))
           , core::memory::make_offset (bs *            8192 )
           )
        , storage::with_range
           ( provider_local_stream_Files.storage()
           , core::memory::make_offset (bs *    0)
           , core::memory::make_offset (bs * 8192)
           )
        , storage::with_range
           ( provider_local_stream_Heap.storage()
           , core::memory::make_offset (bs *    0)
           , core::memory::make_offset (bs * 8192)
           )
        , storage::with_range
           ( provider_local_stream_SHMEM.storage()
           , core::memory::make_offset (bs *    0)
           , core::memory::make_offset (bs * (8192-(6*8192-47000)))
           )
        }
      );

    // 0      1             3      4      5      6      7
    // 0 tf   8 tH          24 lF  32 lH  40 lS  48 tS  56
    // [--)                                   [-)[-----)
    //    ^2000                               ^47000

    ASSERT_EQ ( blocks.number_of_blocks()
              , block::make_count (2000 + (6*8192-47000) + 8192)
              );

    ASSERT_EQ
      ( blocks.blocks()
      , ( std::list<block::Range>
          { block::make_range (block::make_id (0), block::make_id (2000))
          , block::make_range (block::make_id (47000), block::make_id (7*8192))
          }
        )
      );

    check_block (0, provider_tcp_Files, 0);
    check_block (1, provider_tcp_Files, 1);
    check_block (49151, provider_local_stream_SHMEM, 8191);
    check_block (50000, provider_tcp_SHMEM, 848);

    // add storage with a size that is too small to fit a single block
    {
      auto const provider_local_stream_SHMEM_too_small_for_a_single_block
        { StoragesProvider< Element
                          , asio::local::stream_protocol
                          , testing::core::storage::implementation::SHMEM
                          >
          { core::memory::make_size (2 << 10)
          , "sS-t"
          , 0
          }
        };
      auto const storage
        {provider_local_stream_SHMEM_too_small_for_a_single_block.storage()};

      auto const [range, unused] {blocks.add (storage)};

      ASSERT_FALSE (range.has_value());
      ASSERT_TRUE (unused.has_value());
      // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
      ASSERT_EQ (unused.value(), storage);
    }

    // add storage with a size that is not a multiple of the block size
    auto const provider_local_stream_SHMEM_with_half_block
      { StoragesProvider< Element
                        , asio::local::stream_protocol
                        , testing::core::storage::implementation::SHMEM
                        >
        { core::memory::make_size (6 << 10)
        , "sS-h"
        , 24 << 20
        }
    };

    {
      auto const storage
        {provider_local_stream_SHMEM_with_half_block.storage()};

      auto const [range, unused] {blocks.add (storage)};

      ASSERT_TRUE (range.has_value());
      ASSERT_TRUE (unused.has_value());
      // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
      ASSERT_EQ (block::begin (range.value()), block::make_id (7*8192));
      // NOLINTNEXTLINE (bugprone-unchecked-optional-access)
      ASSERT_EQ
        ( unused.value()
        , storage::with_range
          ( storage
          , core::memory::make_offset (bs)
          , core::memory::make_offset (bs + bs/2)
          )
        );
    }

    ASSERT_EQ
      ( blocks.blocks()
      , ( std::list<block::Range>
          { block::make_range (block::make_id (0), block::make_id (2000))
          , block::make_range (block::make_id (47000), block::make_id (7*8192+1))
          }
        )
      );

    // publish the meta data
    auto io_context_meta_data_provider
      { rpc::ScopedRunningIOContext
          { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };

    using MetaDataProtocol = TypeParam;
    auto meta_data_provider
      { meta_data::Provider<MetaDataProtocol>
        { typename MetaDataProtocol::endpoint{}
        , io_context_meta_data_provider
        , std::addressof (blocks)
        }
      };

    // Read/WriteBlock
    // 1. set up a local buffer to store blocks
    // 2. set up and use a meta data client to get the location of the block
    // 3. read/write blocks using a transport clients created in a callback
    using BufferStorage = testing::core::storage::implementation::Heap;
    using SupportedBufferStorageImplementations
      = util::type::List<typename BufferStorage::Storage>
      ;
    auto testing_buffer_storage {BufferStorage {"buffer_storage"}};
    auto buffer_storages
      {core::Storages<SupportedBufferStorageImplementations>{}};
    auto const buffer_storage
      { core::make_unique_storage<typename BufferStorage::Storage>
          ( std::addressof (buffer_storages)
          , testing_buffer_storage.parameter_create()
          )
      };
    auto const buffer_segment
      { core::storage::make_unique_segment<typename BufferStorage::Storage>
          ( std::addressof (buffer_storages)
          , buffer_storage->id()
          , core::memory::make_size (block_size)
          , testing_buffer_storage.parameter_segment_create()
          , testing_buffer_storage.parameter_segment_remove()
          )
      };

    auto buffer
      { Buffer<SupportedBufferStorageImplementations>
        { std::addressof (buffer_storages)
        , buffer_storage->id()
        , core::storage::make_parameter
            (testing_buffer_storage.parameter_chunk_description())
        , buffer_segment->id()
        , core::memory::make_range
           ( core::memory::make_offset (0)
           , core::memory::make_size (block_size)
           )
        , block_size
        }
      };

    auto io_context_transport_client
      { rpc::ScopedRunningIOContext
          { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };

    // \todo re-implement cache!?
    auto transport_clients
      { [&]<util::ASIO::is_protocol Protocol>
          (util::ASIO::Connectable<Protocol> provider_connectable)
        {
          return core::transport::implementation::ASIO::Client
              < Protocol
              , rpc::access_policy::Sequential
              , SupportedBufferStorageImplementations
              >
            { io_context_transport_client
            , provider_connectable
            , std::addressof (buffer_storages)
            };
        }
      };

    auto io_context_meta_data_client
      { rpc::ScopedRunningIOContext
          { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };

    util::ASIO::run
      ( util::ASIO::make_connectable (meta_data_provider.local_endpoint())
      , [&]<util::ASIO::is_protocol MetaDataProviderProtocol>
          (util::ASIO::Connectable<MetaDataProviderProtocol> provider)
        {
          auto meta_data_client
            { meta_data::Client< MetaDataProviderProtocol
                               , rpc::access_policy::Concurrent
                               >
                { io_context_meta_data_client
                , provider
                }
            };

          auto reader
            { make_reader<decltype (transport_clients)&>
                ( meta_data_client
                , transport_clients
                )
            };

          auto const block
            { buffer.block
                ( Buffer<SupportedBufferStorageImplementations>
                    ::InterruptionContext{}
                , std::chrono::steady_clock::now() + std::chrono::seconds (5)
                )
            };

          auto const block_id {49494};
          ASSERT_EQ
            ( core::memory::make_size (block_size)
            , reader (block::make_id (block_id), block->address()).get()
            );

          auto expected_values {std::vector<Element> (bs / sizeof (Element))};
          std::iota ( std::begin (expected_values), std::end (expected_values)
                    , 2 * elements_per_storage
                    + (block_id % 8192) * (bs / sizeof (Element))
                    );

          {
            auto const chunk
              {block->template chunk<core::chunk::access::Const>()};
            ASSERT_THAT ( core::as<Element const> (chunk)
                        , ::testing::ElementsAreArray (expected_values)
                        );
          }

          {
            auto const chunk
              {block->template chunk<core::chunk::access::Mutable>()};
            auto elements {as<Element> (chunk)};
            std::ranges::reverse (elements);
          }

          auto writer
            { make_writer<decltype (transport_clients)&>
                ( meta_data_client
                , transport_clients
                )
            };

          ASSERT_EQ
            ( core::memory::make_size (block_size)
            , writer (block::make_id (block_id), block->address()).get()
            );

          // overlap read with local computation
          {
            auto read {reader (block::make_id (block_id), block->address())};
            std::ranges::reverse (expected_values);
            ASSERT_EQ
              ( core::memory::make_size (block_size)
              , read.get()
              );
          }

          {
            auto const chunk
              {block->template chunk<core::chunk::access::Const>()};
            ASSERT_THAT ( core::as<Element const> (chunk)
                        , ::testing::ElementsAreArray (expected_values)
                        );
          }
        }
      );
  }
}
