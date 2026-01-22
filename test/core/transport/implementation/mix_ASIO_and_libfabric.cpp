// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstddef>
#include <cstdlib>
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
#include <mcs/core/transport/implementation/ASIO/Client.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
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
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/bool.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <span>

namespace mcs::testing::core::transport::implementation::libfabric
{
  // A mixed ASIO/libfabric provider that owns a chunk of data.
  //
  template< rpc::is_protocol LibfabricControlProtocol
          , rpc::is_protocol AsioControlAndTransportProtocol
          , typename TestingStorage
          >
    struct Provider
  {
    Provider
      ( mcs::core::memory::Size size
      , rpc::ScopedRunningIOContext& libfabric_control_io_context
      , rpc::ScopedRunningIOContext& asio_control_and_transport_io_context
      )
        : _size {size}
        , _libfabric_provider
          { libfabric_control_io_context
          , _libfabric_protocol_state.local_endpoint()
          , mcs::core::transport::implementation::libfabric::libfabric::Interface
            { mcs::core::transport::implementation::libfabric::libfabric::Provider {"tcp"}
            , mcs::core::transport::implementation::libfabric::libfabric::Domain {"lo"}
            }
          , std::addressof (_storages)
          }
        , _asio_provider
          { asio_control_and_transport_io_context
          , _asio_protocol_state.local_endpoint()
          , std::addressof (_storages)
          }
    {}

    [[nodiscard]] auto libfabric_connection_information() const
    {
      return _libfabric_provider.connection_information();
    }
    [[nodiscard]] auto asio_connection_information() const
    {
      return _asio_provider.connection_information();
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

    testing::RPC::ProtocolState<LibfabricControlProtocol>
      _libfabric_protocol_state
        { "PF"
        };
    testing::RPC::ProtocolState<AsioControlAndTransportProtocol>
      _asio_protocol_state
        { "PA"
        };

    mcs::core::transport::implementation::libfabric::Provider
      < LibfabricControlProtocol
      , mcs::core::transport::implementation::libfabric::transporter::rdm::Provider
      , SupportedStorageImplementations
      > _libfabric_provider
      ;

    mcs::core::transport::implementation::ASIO::Provider
      < AsioControlAndTransportProtocol
      , SupportedStorageImplementations
      > _asio_provider
      ;
  };
}

namespace mcs::testing::core::transport::implementation::libfabric
{
  // A mixed ASIO/libfabric provider that owns a chunk of data.
  //
  template< rpc::is_protocol LibfabricControlProtocol
          , rpc::is_protocol AsioControlAndTransportProtocol
          , typename TestingStorage
          >
    struct Client
  {
    Client
      ( mcs::core::memory::Size size
      , mcs::rpc::ScopedRunningIOContext& libfabric_control_io_context
      , mcs::rpc::ScopedRunningIOContext& asio_control_and_transport_io_context
      , mcs::core::transport::implementation::libfabric::provider::ConnectionInformation<LibfabricControlProtocol>
          libfabric_connection_information
      , mcs::util::ASIO::Connectable<AsioControlAndTransportProtocol>
          asio_connection_information
      , std::size_t client_id
      )
        : _size {size}
        , _testing_storage {fmt::format ("C-{}", client_id)}
        , _libfabric_client
          { libfabric_control_io_context
          , libfabric_connection_information
          , mcs::core::transport::implementation::libfabric::libfabric::Interface
            { mcs::core::transport::implementation::libfabric::libfabric::Provider {"tcp"}
            , mcs::core::transport::implementation::libfabric::libfabric::Domain {"lo"}
            }
          , std::addressof (_storages)
          }
       , _asio_client
         { asio_control_and_transport_io_context
         , asio_connection_information
         , std::addressof (_storages)
         }
    {}

    [[nodiscard]] auto libfabric_memory_get
      ( mcs::core::transport::Address source
      , mcs::core::memory::Size size
      )
    {
      return _libfabric_client.memory_get (_address, source, size);
    }
    [[nodiscard]] auto asio_memory_get
      ( mcs::core::transport::Address source
      , mcs::core::memory::Size size
      )
    {
      return _asio_client.memory_get (_address, source, size);
    }
    [[nodiscard]] auto libfabric_memory_put
      ( mcs::core::transport::Address destination
      , mcs::core::memory::Size size
      )
    {
      return _libfabric_client.memory_put (_address, destination, size);
    }
    [[nodiscard]] auto asio_memory_put
      ( mcs::core::transport::Address destination
      , mcs::core::memory::Size size
      )
    {
      return _asio_client.memory_put (_address, destination, size);
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
      < LibfabricControlProtocol
      , mcs::rpc::access_policy::Exclusive
      , mcs::core::transport::implementation::libfabric::transporter::rdm::Client
      , SupportedStorageImplementations
      > _libfabric_client
      ;
    mcs::core::transport::implementation::ASIO::Client
      < AsioControlAndTransportProtocol
      , mcs::rpc::access_policy::Exclusive
      , SupportedStorageImplementations
      > _asio_client
      ;
  };
}

namespace mcs::core
{
  template< rpc::is_protocol _ProtocolLibfabricControl
          , rpc::is_protocol _ProtocolAsioControlAndTransport
          , typename _ProviderStorage
          , typename _ClientStorage
          >
    struct ProtocolsAndStorages
  {
    using ProtocolLibfabricControl = _ProtocolLibfabricControl;
    using ProtocolAsioControlAndTransport = _ProtocolAsioControlAndTransport;
    using ProviderStorage = _ProviderStorage;
    using ClientStorage = _ClientStorage;
  };

  namespace TestingStorage = testing::core::storage::implementation;

  // \todo TestingStorage::Virtual
  using StoragePairs = ::testing::Types
    < ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::Files, TestingStorage::Files>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::Files, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::Files, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::Heap, TestingStorage::Files>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::Heap, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::Heap, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::Files>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::ip::tcp, asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::SHMEM>

    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::Files, TestingStorage::Files>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::Files, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::Files, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::Files>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::Files>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::ip::tcp, asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::SHMEM>

    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::Files, TestingStorage::Files>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::Files, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::Files, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::Heap, TestingStorage::Files>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::Heap, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::Heap, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::Files>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::ip::tcp, TestingStorage::SHMEM, TestingStorage::SHMEM>

    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::Files, TestingStorage::Files>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::Files, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::Files, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::Files>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::Heap, TestingStorage::SHMEM>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::Files>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::Heap>
    , ProtocolsAndStorages<asio::local::stream_protocol, asio::local::stream_protocol, TestingStorage::SHMEM, TestingStorage::SHMEM>
    >;

  template<class> struct MCSTransportMixAsioAndLibfabricT
    : public testing::random::Test
  {};

  TYPED_TEST_SUITE (MCSTransportMixAsioAndLibfabricT, StoragePairs);
}

namespace mcs::core
{
  TYPED_TEST (MCSTransportMixAsioAndLibfabricT, to_mix_ASIO_and_libfabric_works)
  {
    using ProtocolLibfabricControl
      = typename TypeParam::ProtocolLibfabricControl
      ;
    using ProtocolAsioControlAndTransport
      = typename TypeParam::ProtocolAsioControlAndTransport
      ;
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

    auto random_bool {testing::random::value<bool>{}};

    auto provider_libfabric_control_io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (testing::random::value<unsigned> {1u, 10u})
          }
        , SIGINT, SIGTERM
        }
      };
    auto provider_asio_control_and_transport_io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (testing::random::value<unsigned> {1u, 10u})
          }
        , SIGINT, SIGTERM
        }
      };

    auto provider
      { testing::core::transport::implementation::libfabric::Provider
          < ProtocolLibfabricControl
          , ProtocolAsioControlAndTransport
          , ProviderStorage
          >
        { size
        , provider_libfabric_control_io_context
        , provider_asio_control_and_transport_io_context
        }
      };

    auto client_libfabric_control_io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (testing::random::value<unsigned> {1u, 10u})
          }
        , SIGINT, SIGTERM
        }
      };
    auto client_asio_control_and_transport_io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
          { std::invoke (testing::random::value<unsigned> {1u, 10u})
          }
        , SIGINT, SIGTERM
        }
      };

    auto clients
      { std::list
          < testing::core::transport::implementation::libfabric::Client
              < ProtocolLibfabricControl
              , ProtocolAsioControlAndTransport
              , ClientStorage
              >
          >
        {}
      };

    for (auto c {std::size_t {0}}; c != number_of_clients; ++c)
    {
      clients.emplace_back
        ( size
        , client_libfabric_control_io_context
        , client_asio_control_and_transport_io_context
        , provider.libfabric_connection_information()
        , provider.asio_connection_information()
        , c
        );
    }

    // all clients gets data from the provider asynchronously in parallel
    // using either libfabric or asio provider/client randomly
    {
      std::generate_n
        ( std::begin (provider.template data<Element>())
        , number_of_elements
        , random_element
        );

      auto gets {std::list<std::future<memory::Size>>{}};

      for (auto& client : clients)
      {
        if (random_bool())
        {
          gets.emplace_back
            ( client.libfabric_memory_get (provider.address(), size)
            );
        }
        else
        {
          gets.emplace_back
            ( client.asio_memory_get (provider.address(), size)
            );
        }
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
    // using either libfabric or asio provider/client randomly
    for (auto& client : clients)
    {
      std::generate_n
        ( std::begin (client.template data<Element>())
        , number_of_elements
        , random_element
        );

      if (random_bool())
      {
        ASSERT_EQ
          ( size
          , client.libfabric_memory_put (provider.address(), size).get()
          );
      }
      else
      {
        ASSERT_EQ
          ( size
          , client.asio_memory_put (provider.address(), size).get()
          );
      }

      ASSERT_THAT
        ( client.template data<Element>()
        , ::testing::ElementsAreArray (provider.template data<Element>())
        );
    }

    // libfabric and asio clients with the same storage puts data to the
    // provider asynchronously in parallel
    {
      auto& client {clients.front()};

      std::generate_n
        ( std::begin (client.template data<Element>())
        , number_of_elements
        , random_element
        );

      auto puts {std::list<std::future<memory::Size>>{}};

      puts.emplace_back
        ( client.asio_memory_put (provider.address(), size)
        );

      puts.emplace_back
        ( client.libfabric_memory_put (provider.address(), size)
        );

      for (auto& put : puts)
      {
        ASSERT_EQ (size, put.get());
      }

      ASSERT_THAT
        ( client.template data<Element>()
        , ::testing::ElementsAreArray (provider.template data<Element>())
        );
    }

    // libfabric and asio clients with different storages puts data to the
    // provider asynchronously in parallel
    {
      ASSERT_NE (std::next (std::begin (clients)), std::end (clients));

      auto& client1 {clients.front()};
      auto& client2 {*std::next (clients.begin())};

      std::generate_n
        ( std::begin (client1.template data<Element>())
        , number_of_elements
        , random_element
        );

      std::generate_n
        ( std::begin (client2.template data<Element>())
        , number_of_elements
        , random_element
        );

      auto puts {std::list<std::future<memory::Size>>{}};

      puts.emplace_back
        ( client1.asio_memory_put (provider.address(), size)
        );

      puts.emplace_back
        ( client2.libfabric_memory_put (provider.address(), size)
        );

      for (auto& put : puts)
      {
        ASSERT_EQ (size, put.get());
      }

      auto const provider_bytes
        { std::as_bytes (provider.template data<Element>())
        };
      auto const client1_bytes
        { std::as_bytes (client1.template data<Element>())
        };
      auto const client2_bytes
        { std::as_bytes (client2.template data<Element>())
        };

      ASSERT_EQ (std::size (provider_bytes), std::size (client1_bytes));
      ASSERT_EQ (std::size (provider_bytes), std::size (client2_bytes));

      for (auto b {std::size_t {0}}; b != std::size (provider_bytes); ++b)
      {
        ASSERT_TRUE
          (  provider_bytes[b] == client1_bytes[b]
          || provider_bytes[b] == client2_bytes[b]
          );
      }
    }
  }
}
