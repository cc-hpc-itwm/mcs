// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <compare>
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <map>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/ASIO/Client.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/Heap.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <span>

namespace mcs::core
{
  namespace
  {
    template<rpc::is_protocol P, typename L, typename R>
      struct ProtocolAndStorages
    {
      using Protocol = P;
      using First = L;
      using Second = R;
    };

    namespace Impl = testing::core::storage::implementation;

    using StoragePairs = ::testing::Types
      < ProtocolAndStorages<asio::ip::tcp, Impl::Files, Impl::Files>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Files, Impl::Heap>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Files, Impl::SHMEM>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Heap, Impl::Files>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Heap, Impl::Heap>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Heap, Impl::SHMEM>
      , ProtocolAndStorages<asio::ip::tcp, Impl::SHMEM, Impl::Files>
      , ProtocolAndStorages<asio::ip::tcp, Impl::SHMEM, Impl::Heap>
      , ProtocolAndStorages<asio::ip::tcp, Impl::SHMEM, Impl::SHMEM>

      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Files>, Impl::Files>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Files>, Impl::Heap>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Files>, Impl::SHMEM>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Heap>, Impl::Files>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Heap>, Impl::Heap>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Heap>, Impl::SHMEM>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::SHMEM>, Impl::Files>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::SHMEM>, Impl::Heap>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::SHMEM>, Impl::SHMEM>

      , ProtocolAndStorages<asio::ip::tcp, Impl::Files, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Files, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Files, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Heap, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Heap, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Heap, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::SHMEM, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::SHMEM, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::SHMEM, Impl::Virtual<Impl::SHMEM>>

      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Files>, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Files>, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Files>, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Heap>, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Heap>, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::Heap>, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::SHMEM>, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::SHMEM>, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::ip::tcp, Impl::Virtual<Impl::SHMEM>, Impl::Virtual<Impl::SHMEM>>

      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Files, Impl::Files>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Files, Impl::Heap>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Files, Impl::SHMEM>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Heap, Impl::Files>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Heap, Impl::Heap>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Heap, Impl::SHMEM>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::SHMEM, Impl::Files>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::SHMEM, Impl::Heap>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::SHMEM, Impl::SHMEM>

      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Files>, Impl::Files>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Files>, Impl::Heap>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Files>, Impl::SHMEM>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>, Impl::Files>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>, Impl::Heap>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>, Impl::SHMEM>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>, Impl::Files>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>, Impl::Heap>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>, Impl::SHMEM>

      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Files, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Files, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Files, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Heap, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Heap, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Heap, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::SHMEM, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::SHMEM, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::SHMEM, Impl::Virtual<Impl::SHMEM>>

      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Files>, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Files>, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Files>, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::Heap>, Impl::Virtual<Impl::SHMEM>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>, Impl::Virtual<Impl::Files>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>, Impl::Virtual<Impl::Heap>>
      , ProtocolAndStorages<asio::local::stream_protocol, Impl::Virtual<Impl::SHMEM>, Impl::Virtual<Impl::SHMEM>>
      >;

    template<class> struct MCSTransportAsio : public testing::random::Test{};
    TYPED_TEST_SUITE (MCSTransportAsio, StoragePairs);

    template< typename Element
            , rpc::is_protocol Protocol
            , typename TestingStorage
            >
      struct StoragesProvider
    {
      using SupportedStorageImplementations
        = util::type::List<typename TestingStorage::Storage>
        ;

      using ProviderImplementation
        = core::transport::implementation::ASIO::Provider
            < Protocol
            , SupportedStorageImplementations
            >
        ;

      template<typename RandomElement, typename Tag>
        StoragesProvider
          ( RandomElement& random_element
          , std::size_t number_of_elements_per_chunk
          , memory::Size number_of_bytes_per_chunk
          , Tag tag
          )
            : _testing_storage {fmt::format ("P-{}", tag)}
            , _number_of_bytes_per_chunk {number_of_bytes_per_chunk}
            , _protocol_state {fmt::format ("P-{}", tag)}
      {
        std::generate_n
          ( _elements.begin()
          , number_of_elements_per_chunk
          , random_element
          );
      }

      [[nodiscard]] auto connection_information
        (
        ) const -> util::ASIO::Connectable<Protocol>
      {
        return _provider.connection_information();
      }

      [[nodiscard]] auto source() const -> transport::Address
      {
        return transport::Address
          { _storage->id()
          , storage::make_parameter
              (_testing_storage.parameter_chunk_description())
          , _segment->id()
          , memory::make_offset (0)
          };
      }

      [[nodiscard]] auto elements() -> std::span<Element const>
      {
        return _elements;
      }

    private:
      core::Storages<SupportedStorageImplementations> _storages{};
      TestingStorage _testing_storage;
      memory::Size _number_of_bytes_per_chunk;
      SupportedStorageImplementations::template wrap
          < UniqueStorage
          , typename TestingStorage::Storage
          > _storage
            { make_unique_storage<typename TestingStorage::Storage>
                ( std::addressof (_storages)
                , _testing_storage.parameter_create()
                )
            };
      SupportedStorageImplementations::template wrap
        < storage::UniqueSegment
        , typename TestingStorage::Storage
        > _segment
          { storage::make_unique_segment<typename TestingStorage::Storage>
              ( std::addressof (_storages)
              , _storage->id()
              , _number_of_bytes_per_chunk
              , _testing_storage.parameter_segment_create()
              , _testing_storage.parameter_segment_remove()
              )
          };
      SupportedStorageImplementations::template wrap
          < Chunk
          , chunk::access::Mutable
          > _chunk
            { _storages.template chunk_description
                  < typename TestingStorage::Storage
                  , chunk::access::Mutable
                  >
                ( _storages.read_access()
                , _storage->id()
                , _testing_storage.parameter_chunk_description()
                , _segment->id()
                , memory::make_range ( memory::make_offset (0)
                                     , _number_of_bytes_per_chunk
                                     )
                )
            };
      std::span<Element> _elements {as<Element> (_chunk)};
      rpc::ScopedRunningIOContext _io_context
        {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM};
      testing::RPC::ProtocolState<Protocol> _protocol_state;
      ProviderImplementation _provider
        { _io_context
        , _protocol_state.local_endpoint()
        , std::addressof (_storages)
        };
    };

    template< typename Element
            , rpc::is_protocol Protocol
            , typename TestingStorage
            , typename StoragesProvider
            >
      struct StoragesClient
    {
      using SupportedClientStorageImplementations
        = util::type::List<typename TestingStorage::Storage>
        ;

      using ProviderImplementation
        = typename StoragesProvider::ProviderImplementation
        ;

      template<typename Tag>
        StoragesClient
          ( util::ASIO::Connectable<Protocol> connection_information
          , memory::Size number_of_bytes_per_chunk
          , Tag tag
          )
            : _testing_storage {fmt::format ("C-{}", tag)}
            , _number_of_bytes_per_chunk {number_of_bytes_per_chunk}
            , _connection_information {connection_information}
      {}

      [[nodiscard]] auto memory_get
        ( transport::Address source
        ) const -> std::future<memory::Size>
      {
        return _client.memory_get
          ( local_address()
          , source
          , _number_of_bytes_per_chunk
          );
      }

      [[nodiscard]] auto memory_put
        ( transport::Address destination
        ) const -> std::future<memory::Size>
      {
        return _client.memory_put
          ( destination
          , local_address()
          , _number_of_bytes_per_chunk
          );
      }

      [[nodiscard]] auto elements() -> std::span<Element const>
      {
        return _elements;
      }

      template<typename Generator>
        auto generate (Generator&& generator)
      {
        auto const chunk
          { typename SupportedClientStorageImplementations::template wrap
              < Chunk
              , chunk::access::Mutable
              >
                { _storages.template chunk_description
                      < typename TestingStorage::Storage
                      , chunk::access::Mutable
                      >
                    ( _storages.read_access()
                    , _storage->id()
                    , _testing_storage.parameter_chunk_description()
                    , _segment->id()
                    , memory::make_range ( memory::make_offset (0)
                                         , _number_of_bytes_per_chunk
                                         )
                    )
                }
          };
        std::ranges::generate
          (as<Element> (chunk), std::forward<Generator> (generator));
      }

    private:
      [[nodiscard]] auto local_address() const -> transport::Address
      {
        return transport::Address
          { _storage->id()
          , storage::make_parameter
              (_testing_storage.parameter_chunk_description())
          , _segment->id()
          , memory::make_offset (0)
          };
      }

      Storages<SupportedClientStorageImplementations> _storages{};
      TestingStorage _testing_storage;
      memory::Size _number_of_bytes_per_chunk;
      SupportedClientStorageImplementations::template wrap
        < UniqueStorage
        , typename TestingStorage::Storage
        > _storage
          { make_unique_storage<typename TestingStorage::Storage>
              ( std::addressof (_storages)
              , _testing_storage.parameter_create()
              )
          };
      SupportedClientStorageImplementations::template wrap
        < storage::UniqueSegment
        , typename TestingStorage::Storage
        > _segment
          { storage::make_unique_segment<typename TestingStorage::Storage>
            ( std::addressof (_storages)
            , _storage->id()
            , _number_of_bytes_per_chunk
            , _testing_storage.parameter_segment_create()
            , _testing_storage.parameter_segment_remove()
            )
        };
      SupportedClientStorageImplementations::template wrap
        < Chunk
        , chunk::access::Const
        > _chunk
          { _storages.template chunk_description
                < typename TestingStorage::Storage
                , chunk::access::Const
                >
              ( _storages.read_access()
              , _storage->id()
              , _testing_storage.parameter_chunk_description()
              , _segment->id()
              , memory::make_range ( memory::make_offset (0)
                                   , _number_of_bytes_per_chunk
                                   )
              )
          };
      std::span<Element const> _elements {as<Element const> (_chunk)};
      rpc::ScopedRunningIOContext _io_context
        {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM};

      util::ASIO::Connectable<Protocol> _connection_information;
      transport::implementation::ASIO::Client
         < Protocol
         , rpc::access_policy::Exclusive
         , SupportedClientStorageImplementations
         >
        _client
          { _io_context
          , _connection_information
          , std::addressof (_storages)
          };
    };
  }

  TYPED_TEST (MCSTransportAsio, memory_get_put_works)
  {
    using Protocol = typename TypeParam::Protocol;
    using TestingStorageProvider = typename TypeParam::First;
    using TestingStorageClient = typename TypeParam::Second;

    using RandomSize = testing::random::value<std::size_t>;
    using Element = int;

    auto const number_of_elements_per_chunk
      { RandomSize { RandomSize::Min {32 << 10}
                   , RandomSize::Max {64 << 10}
                   }()
      };
    auto const number_of_bytes_per_chunk
      {memory::make_size (number_of_elements_per_chunk * sizeof (Element))};

    auto random_element {testing::random::value<Element>{}};

    auto random_multiplicity { RandomSize { RandomSize::Min {1}
                                          , RandomSize::Max {3}
                                          }
                             };

    auto const number_of_providers {random_multiplicity()};

    // provide some storages with one segment each
    using Provider = StoragesProvider< Element
                                     , Protocol
                                     , TestingStorageProvider
                                     >;

    auto providers {std::list<Provider>{}};

    for (auto p {std::size_t {0}}; p != number_of_providers; ++p)
    {
      providers.emplace_back
        ( random_element
        , number_of_elements_per_chunk
        , number_of_bytes_per_chunk
        , p
        );
    }

    // for each provided storage create a number of clients
    using Client = StoragesClient< Element
                                 , Protocol
                                 , TestingStorageClient
                                 , Provider
                                 >;

    struct ProviderIndex
    {
      int id;
      typename std::list<Provider>::iterator provider;

      constexpr auto operator<=> (ProviderIndex const& other) const noexcept
      {
        return id <=> other.id;
      }
    };

    auto clients_by_provider {std::map<ProviderIndex, std::list<Client>>{}};

    {
      auto p {0};

      for ( auto provider {std::begin (providers)}
          ; provider != std::end (providers)
          ; ++provider, ++p
          )
      {
        auto const number_of_clients {random_multiplicity()};

        for (auto c {std::size_t {0}}; c != number_of_clients; ++c)
        {
          clients_by_provider[{p, provider}].emplace_back
            ( provider->connection_information()
            , number_of_bytes_per_chunk
            , fmt::format ("{}-{}", p, c)
            );
        }
      }
    }

    // let all clients read, asynchronously and in parallel, read from
    // their providers, they all end up with tha same copy of data
    {
      auto gets {std::list<std::future<memory::Size>>{}};

      for (auto& [provider_index, clients] : clients_by_provider)
      {
        for (auto& client : clients)
        {
          gets.emplace_back
            (client.memory_get (provider_index.provider->source()));
        }
      }

      for (auto& get : gets)
      {
        ASSERT_EQ (number_of_bytes_per_chunk, get.get());
      }

      for (auto& [provider_index, clients] : clients_by_provider)
      {
        for (auto& client : clients)
        {
          ASSERT_THAT
            ( client.elements()
            , ::testing::ElementsAreArray (provider_index.provider->elements())
            );
        }
      }
    }

    // let all clients, synchrounous and in sequence, write to their
    // provider, multiple clients per provider do overwrite the data
    for (auto& [provider_index, clients] : clients_by_provider)
    {
      for (auto& client : clients)
      {
        client.generate (random_element);

        ASSERT_EQ ( number_of_bytes_per_chunk
                  , client.memory_put (provider_index.provider->source()).get()
                  );

        ASSERT_THAT
          ( client.elements()
          , ::testing::ElementsAreArray (provider_index.provider->elements())
          );
      }
    }

    // let one client per provider, asynchronous and in parallel,
    // write to their provider. Note: All clients _could_ write in
    // parallel, however, the result would be some undefined
    // interleaving of all of their data
    {
      auto puts {std::list<std::future<memory::Size>>{}};

      for (auto& [provider_index, clients] : clients_by_provider)
      {
        auto& client {clients.front()};

        client.generate (random_element);

        puts.emplace_back
          (client.memory_put (provider_index.provider->source()));
      }

      for (auto& put : puts)
      {
        ASSERT_EQ (number_of_bytes_per_chunk, put.get());
      }

      for (auto& [provider_index, clients] : clients_by_provider)
      {
        ASSERT_THAT
          ( clients.front().elements()
          , ::testing::ElementsAreArray (provider_index.provider->elements())
          );
      }
    }
  }
}
