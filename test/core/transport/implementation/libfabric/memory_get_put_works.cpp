// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstddef>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/libfabric/Client.hpp>
#include <mcs/core/transport/implementation/libfabric/Provider.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Domain.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Provider.hpp>
#include <mcs/core/transport/implementation/libfabric/provider/ConnectionInformation.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Client.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Provider.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>

namespace mcs::testing::core::transport::implementation::libfabric
{
  // A libfabric provider that owns a chunk of data.
  //
  template<rpc::is_protocol ControlProtocol, typename TestingStorage>
    struct Provider
  {
    Provider
      ( mcs::core::memory::Size size
      , rpc::ScopedRunningIOContext& io_context
      )
        : _size {size}
        , _provider
          { io_context
          , _protocol_state.local_endpoint()
          , mcs::core::transport::implementation::libfabric::libfabric::Interface
            { mcs::core::transport::implementation::libfabric::libfabric::Provider {"tcp"}
            , mcs::core::transport::implementation::libfabric::libfabric::Domain {"lo"}
            }
          , std::addressof (_storages)
          }
    {}

    [[nodiscard]] auto connection_information() const
    {
      return _provider.connection_information();
    }
    template<typename T>
      [[nodiscard]] auto data() const
    {
      return mcs::core::as<T> (_chunk);
    }
    [[nodiscard]] auto address() const
    {
      return _address;
    }

  private:
    using Storage = TestingStorage::Storage;
    using SupportedStorageImplementations = util::type::List<Storage>;

    mcs::core::memory::Size _size;

    mcs::core::Storages<SupportedStorageImplementations> _storages{};

    TestingStorage _testing_storage {fmt::format ("P")};

    SupportedStorageImplementations::template wrap
        < mcs::core::UniqueStorage
        , Storage
        > _storage
      { mcs::core::make_unique_storage<Storage>
          ( std::addressof (_storages)
          , _testing_storage.parameter_create()
          )
      };

    SupportedStorageImplementations::template wrap
        < mcs::core::storage::UniqueSegment
        , Storage
        > _segment
      { mcs::core::storage::make_unique_segment<Storage>
          ( std::addressof (_storages)
          , _storage->id()
          , _size
          , _testing_storage.parameter_segment_create()
          , _testing_storage.parameter_segment_remove()
          )
      };

    SupportedStorageImplementations::template wrap
      < mcs::core::Chunk
      , mcs::core::chunk::access::Mutable
      > _chunk
        { _storages.read_access().template invoke<Storage>
          ( _storage->id()
          , [&] (auto const& storage_implementation)
            {
              return storage_implementation
                . template chunk_description
                    < mcs::core::chunk::access::Mutable
                    >
                  ( _testing_storage.parameter_chunk_description()
                  , _segment->id()
                  , mcs::core::memory::make_range
                    ( mcs::core::memory::make_offset (0)
                    , _size
                    )
                  )
                ;
            }
          )
        };

    mcs::core::transport::Address _address
      { _storage->id()
        , mcs::core::storage::make_parameter
          ( typename Storage::Parameter::Chunk::Description{}
          )
      , _segment->id()
      , mcs::core::memory::make_offset (0)
      };

    testing::RPC::ProtocolState<ControlProtocol> _protocol_state
      { "P"
      };

    mcs::core::transport::implementation::libfabric::Provider
      < ControlProtocol
      , mcs::core::transport::implementation::libfabric::transporter::rdm::Provider
      , SupportedStorageImplementations
      > _provider
      ;
  };
}

namespace mcs::testing::core::transport::implementation::libfabric
{
  // A libfabric provider that owns a chunk of data.
  //
  template<rpc::is_protocol ControlProtocol, typename TestingStorage>
    struct Client
  {
    Client
      ( mcs::core::memory::Size size
      , mcs::rpc::ScopedRunningIOContext& io_context
      , mcs::core::transport::implementation::libfabric::provider::ConnectionInformation<ControlProtocol>
          connection_information
      , std::size_t client_id
      )
        : _size {size}
        , _testing_storage {fmt::format ("C-{}", client_id)}
        , _client
          { io_context
          , connection_information
          , mcs::core::transport::implementation::libfabric::libfabric::Interface
            { mcs::core::transport::implementation::libfabric::libfabric::Provider {"tcp"}
            , mcs::core::transport::implementation::libfabric::libfabric::Domain {"lo"}
            }
          , std::addressof (_storages)
          }
    {}

    [[nodiscard]] auto memory_get
      ( mcs::core::transport::Address source
      , mcs::core::memory::Size size
      )
    {
      return _client.memory_get (_address, source, size);
    }
    [[nodiscard]] auto memory_put
      ( mcs::core::transport::Address destination
      , mcs::core::memory::Size size
      )
    {
      return _client.memory_put (_address, destination, size);
    }
    template<typename T>
      [[nodiscard]] auto data() const
    {
      return mcs::core::as<T> (_chunk);
    }

  private:
    using Storage = TestingStorage::Storage;
    using SupportedStorageImplementations = mcs::util::type::List<Storage>;

    mcs::core::memory::Size _size;

    mcs::core::Storages<SupportedStorageImplementations> _storages{};

    TestingStorage _testing_storage;

    SupportedStorageImplementations::template wrap
        < mcs::core::UniqueStorage
        , Storage
        > _storage
      { mcs::core::make_unique_storage<Storage>
          ( std::addressof (_storages)
          , _testing_storage.parameter_create()
          )
      };

    SupportedStorageImplementations::template wrap
      < mcs::core::storage::UniqueSegment
      , Storage
      > _segment
        { mcs::core::storage::make_unique_segment<Storage>
            ( std::addressof (_storages)
            , _storage->id()
            , _size
            , _testing_storage.parameter_segment_create()
            , _testing_storage.parameter_segment_remove()
            )
        };

    SupportedStorageImplementations::template wrap
      < mcs::core::Chunk
      , mcs::core::chunk::access::Mutable
      > _chunk
        { _storages.read_access().template invoke<Storage>
          ( _storage->id()
          , [&] (auto const& storage_implementation)
            {
              return storage_implementation
                . template chunk_description
                    < mcs::core::chunk::access::Mutable
                    >
                  ( _testing_storage.parameter_chunk_description()
                  , _segment->id()
                  , mcs::core::memory::make_range
                    ( mcs::core::memory::make_offset (0)
                    , _size
                    )
                  )
                ;
            }
          )
        };

    mcs::core::transport::Address _address
      { _storage->id()
      , mcs::core::storage::make_parameter
          (_testing_storage.parameter_chunk_description())
      , _segment->id()
      , mcs::core::memory::make_offset (0)
      };

    mcs::core::transport::implementation::libfabric::Client
      < ControlProtocol
      , mcs::rpc::access_policy::Exclusive
      , mcs::core::transport::implementation::libfabric::transporter::rdm::Client
      , SupportedStorageImplementations
      > _client
      ;
  };
}

namespace mcs::core
{
  template< rpc::is_protocol _Protocol
          , typename _ProviderStorage
          , typename _ClientStorage
          >
    struct ProtocolAndStorages
  {
    using Protocol = _Protocol;
    using ProviderStorage = _ProviderStorage;
    using ClientStorage = _ClientStorage;
  };

  namespace TestingStorage = testing::core::storage::implementation;

  // \todo TestingStorage::Virtual
  using StoragePairs = ::testing::Types
    < ProtocolAndStorages<asio::ip::tcp, TestingStorage::Files, TestingStorage::Files>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::Files, TestingStorage::Heap>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::Files, TestingStorage::SHMEM>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::Heap, TestingStorage::Files>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::Heap, TestingStorage::Heap>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::Heap, TestingStorage::SHMEM>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::Files>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::Heap>
    , ProtocolAndStorages<asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::SHMEM>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::Files, TestingStorage::Files>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::Files, TestingStorage::Heap>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::Files, TestingStorage::SHMEM>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::Files>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::Heap>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::SHMEM>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::Files>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::Heap>
    , ProtocolAndStorages<asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::SHMEM>
    >;

  template<class> struct MCSTransportLibfabricT
    : public testing::random::Test
  {};

  TYPED_TEST_SUITE (MCSTransportLibfabricT, StoragePairs);
}

namespace mcs::core
{
  TYPED_TEST (MCSTransportLibfabricT, memory_get_put_works)
  {
    using Protocol = typename TypeParam::Protocol;
    using ProviderStorage = typename TypeParam::ProviderStorage;
    using ClientStorage = typename TypeParam::ClientStorage;

    using Element = long;

    auto const number_of_elements
      {std::invoke (testing::random::value<unsigned> {0u, 1u << 17u})};
    auto const size_in_bytes {number_of_elements * sizeof (Element)};
    auto const size {memory::make_size (size_in_bytes)};
    auto const number_of_clients
      { 2u * std::invoke (testing::random::value<unsigned> {1u, 10u})
      };

    auto random_element {testing::random::value<Element>{}};

    auto provider_io_context
      { rpc::ScopedRunningIOContext
        { rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (testing::random::value<unsigned> {1u, 10u})
          }
        , SIGINT, SIGTERM
        }
      };

    auto provider
      { testing::core::transport::implementation::libfabric::Provider
          < Protocol
          , ProviderStorage
          >
        { size
        , provider_io_context
        }
      };

    auto client_io_context
      { rpc::ScopedRunningIOContext
        { rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (testing::random::value<unsigned> {1u, 10u})
          }
        , SIGINT, SIGTERM
        }
      };

    auto clients
      { std::list
          < testing::core::transport::implementation::libfabric::Client
              < Protocol
              , ClientStorage
              >
          >
        {}
      };

    for (auto c {std::size_t {0}}; c != number_of_clients; ++c)
    {
      clients.emplace_back
        ( size
        , client_io_context
        , provider.connection_information()
        , c
        );
    }

    // all clients gets data from the provider asynchronously in parallel
    {
      std::generate_n
        ( std::begin (provider.template data<Element>())
        , number_of_elements
        , random_element
        );

      auto gets {std::list<std::future<memory::Size>>{}};

      for (auto& client : clients)
      {
        gets.emplace_back (client.memory_get (provider.address(), size));
      }

      for (auto& get : gets)
      {
        ASSERT_EQ (size, get.get());
      }

      for (auto& client : clients)
      {
        ASSERT_THAT
          ( client.template data<Element>()
          , ::testing::ElementsAreArray (provider.template data<Element>())
          );
      }
    }

    // each client puts data to the provider synchronously in sequence
    for (auto& client : clients)
    {
      std::generate_n
        ( std::begin (client.template data<Element>())
        , number_of_elements
        , random_element
        );

      ASSERT_EQ
        ( size
        , client.memory_put (provider.address(), size).get()
        );

      ASSERT_THAT
        ( client.template data<Element>()
        , ::testing::ElementsAreArray (provider.template data<Element>())
        );
    }
  }
}
