// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cassert>
#include <chrono>
#include <concepts>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <future>
#include <iov/iov.hpp>
#include <iterator>
#include <map>
#include <mcs/IOV_Meta.hpp>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/ASIO/Client.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/Parameter.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/iov_backend/detail/Bitmap.hpp>
#include <mcs/iov_backend/invoke_and_throw_on_unexpected.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/member_AUTO.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/read/read.hpp>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <semaphore>
#include <shared_mutex>
#include <span>
#include <sstream>
#include <stdexcept>
#include <sys/types.h>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

[[nodiscard]] auto iov::iov_backend_ops() -> iov::BackendOps;

namespace
{
  [[nodiscard]] auto backend_init
    ( std::filesystem::path config
    ) -> iov::BackendOps::State
    ;

  auto backend_destroy
    ( iov::BackendOps::State
    ) -> void
    ;

  [[nodiscard]] auto backend_read
    ( iov::BackendOps::State
    , iov::ReadParam
    ) noexcept -> iov::Request<iov::ReadRequestData>
    ;

  [[nodiscard]] auto backend_write
    ( iov::BackendOps::State
    , iov::WriteParam
    ) noexcept -> iov::Request<iov::WriteRequestData>
    ;

  [[nodiscard]] auto backend_allocate
    ( iov::BackendOps::State
    , std::size_t
    ) -> std::span<std::byte>
    ;

  auto backend_free
    ( iov::BackendOps::State
    , std::span<std::byte>
    ) -> void
    ;

  [[nodiscard]] auto backend_collection_open
    ( iov::BackendOps::State
    , iov::CollectionOpenParam
    ) noexcept -> iov::expected<void, iov::ErrorReason>
    ;

  [[nodiscard]] auto backend_collection_close
    ( iov::BackendOps::State
    , iov::CollectionCloseParam
    ) noexcept -> iov::expected<void, iov::ErrorReason>
    ;

  [[nodiscard]] auto backend_collection_delete
    ( iov::BackendOps::State
    , iov::DeleteParam
    ) noexcept -> iov::Request<iov::DeleteRequestData>
    ;
}

auto iov::iov_backend_ops() -> iov::BackendOps
{
  return iov::BackendOps
    { std::addressof (backend_init)
    , std::addressof (backend_destroy)

    , std::addressof (backend_allocate)
    , std::addressof (backend_free)

    , nullptr // backend_batch

    , std::addressof (backend_read)
    , std::addressof (backend_write)

    , std::addressof (backend_collection_open)
    , std::addressof (backend_collection_close)
    , std::addressof (backend_collection_delete)
    };
}

namespace
{
  template< mcs::iov_backend::is_supported_storage_implementation
              StorageImplementation
          >
    struct Buffer
  {
    Buffer
      ( mcs::core::Storages<mcs::iov_backend::SupportedStorageImplementations>&
      , mcs::core::memory::Size
      );

    [[nodiscard]] auto address
      ( mcs::core::memory::Offset
      ) const -> mcs::core::transport::Address
      ;

    template<mcs::core::chunk::is_access Access>
      [[nodiscard]] auto data (mcs::core::memory::Range range) const
        -> typename Access::template Span<std::byte>
      ;

  private:
    mcs::core::Storages<mcs::iov_backend::SupportedStorageImplementations>&
      _storages;
    mcs::core::memory::Size _size;

    mcs::iov_backend::SupportedStorageImplementations::template wrap
      < mcs::core::UniqueStorage
      , StorageImplementation
      > _storage
        { mcs::core::make_unique_storage<StorageImplementation>
            ( std::addressof (_storages)
            , typename StorageImplementation::Parameter::Create
                { mcs::core::storage::MaxSize::Limit {_size}
                }
            )
        };

    mcs::iov_backend::SupportedStorageImplementations::template wrap
      < mcs::core::storage::UniqueSegment
      , StorageImplementation
      > _segment
        { mcs::core::storage::make_unique_segment<StorageImplementation>
            ( std::addressof (_storages)
            , _storage->id()
            , _size
            )
        };
  };
}

namespace mcs::iov_backend
{
  namespace
  {
    struct AcquiredBuffer
    {
      explicit AcquiredBuffer
        ( detail::Bitmap*
        , detail::Bitmap::InterruptionContext const&
        , Parameter::IndirectCommunication const&
        );

      [[nodiscard]] auto offset() const noexcept -> core::memory::Offset;

    private:
      struct Deleter
      {
        explicit Deleter (detail::Bitmap*) noexcept;
        auto operator() (detail::Bitmap::Index*) const noexcept -> void;

      private:
        detail::Bitmap* _bitmap;
      };

      Parameter::IndirectCommunication const& _indirect_communication;
      std::unique_ptr<detail::Bitmap::Index, Deleter> _bit;
    };
  }
}


// \todo move somewhere? keep it here?
//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/Provider.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/MapWithHitMissCallbacks.hpp>
#include <variant>

namespace mcs::core::transport::implementation::ASIO
{
  template< rpc::is_access_policy AccessPolicy
          , typename Executor
          , typename Hit
          , typename Miss
          >
    struct ClientsCache
  {
    ClientsCache
      ( Executor&
      , util::not_null
          < core::Storages<iov_backend::SupportedStorageImplementations>
          >
      , Hit&&
      , Miss&&
      );

    // \todo independent clients storages!?
    // \todo use HeterogeneousMap
    using AnyClient = std::variant
      < Client< asio::ip::tcp
              , AccessPolicy
              , iov_backend::SupportedStorageImplementations
              >
      , Client< asio::local::stream_protocol
              , AccessPolicy
              , iov_backend::SupportedStorageImplementations
              >
      >;

   auto operator() (util::ASIO::AnyConnectable) -> AnyClient;

  private:
    Executor& _executor;
    util::not_null
      < core::Storages<iov_backend::SupportedStorageImplementations>
      > _storages;
    Hit _hit;
    Miss _miss;

    util::ConcurrentMapWithHitMissCallbacks
      < util::ASIO::AnyConnectable
      , AnyClient
      > _clients
    ;
  };

  template< rpc::is_access_policy AccessPolicy
          , typename Executor
          , typename Hit
          , typename Miss
          >
    auto make_clients_cache
      ( Executor&
      , util::not_null
          < core::Storages<iov_backend::SupportedStorageImplementations>
          >
      , Hit&&
      , Miss&&
      );

  template<rpc::is_access_policy AccessPolicy>
    struct MapWithHitMissCallbacksTraits
  {
    using Key = util::ASIO::AnyConnectable;
    using Value = std::variant
      < Client< asio::ip::tcp
              , AccessPolicy
              , iov_backend::SupportedStorageImplementations
              >
      , Client< asio::local::stream_protocol
              , AccessPolicy
              , iov_backend::SupportedStorageImplementations
              >
      >;
    using Map = std::unordered_map<Key, Value>;
  };
}

#include <utility>

namespace mcs::core::transport::implementation::ASIO
{
  template< rpc::is_access_policy AccessPolicy
          , typename Executor
          , typename Hit
          , typename Miss
          >
    ClientsCache<AccessPolicy, Executor, Hit, Miss>::ClientsCache
      ( Executor& executor
      , util::not_null
          < core::Storages<iov_backend::SupportedStorageImplementations>
          > storages
      , Hit&& hit
      , Miss&& miss
      )
        : _executor {executor}
        , _storages {storages}
        , _hit {std::forward<Hit> (hit)}
        , _miss {std::forward<Miss> (miss)}
  {}

  template< rpc::is_access_policy AccessPolicy
          , typename Executor
          , typename Hit
          , typename Miss
          >
      auto ClientsCache<AccessPolicy, Executor, Hit, Miss>::operator()
        ( util::ASIO::AnyConnectable provider
        ) -> AnyClient
  {
    return _clients.at_or_create
      ( provider
      , _hit
      , _miss
      , [&]
        {
          return util::ASIO::run
            ( provider
            , [&]<util::ASIO::is_protocol Protocol>
                ( util::ASIO::Connectable<Protocol> provider_connectable
                ) -> AnyClient
              {
                return core::transport::implementation::ASIO::Client
                    < Protocol
                    , rpc::access_policy::Sequential
                    , iov_backend::SupportedStorageImplementations
                    >
                  { _executor
                  , provider_connectable
                  , _storages
                  };
              }
            );
        }
      );
  }

  template< rpc::is_access_policy AccessPolicy
          , typename Executor
          , typename Hit
          , typename Miss
          >
    auto make_clients_cache
      ( Executor& executor
      , util::not_null
          < core::Storages<iov_backend::SupportedStorageImplementations>
          > storages
      , Hit&& hit
      , Miss&& miss
      )
  {
    return ClientsCache<AccessPolicy, Executor, Hit, Miss>
      { executor
      , storages
      , std::forward<Hit> (hit)
      , std::forward<Miss> (miss)
      };
  }
}

//////////////////////////////////////////////////////////////////////////////


namespace
{
  struct State
  {
    explicit State (mcs::iov_backend::Parameter);

    [[nodiscard]] auto read
      ( iov::ReadParam
      ) noexcept -> iov::Request<iov::ReadRequestData>
      ;
    [[nodiscard]] auto write
      ( iov::WriteParam
      ) noexcept -> iov::Request<iov::WriteRequestData>
      ;

    [[nodiscard]] auto allocate (std::size_t) -> std::span<std::byte>;
    auto free (std::span<std::byte>) -> void;

    [[nodiscard]] auto collection_open
      ( iov::CollectionOpenParam
      ) noexcept -> iov::expected<void, iov::ErrorReason>
      ;

    [[nodiscard]] auto collection_close
      ( iov::CollectionCloseParam
      ) noexcept -> iov::expected<void, iov::ErrorReason>
      ;

    [[nodiscard]] auto collection_delete
      ( iov::DeleteParam
      ) noexcept -> iov::Request<iov::DeleteRequestData>
      ;

    ~State();
    State (State const&) = delete;
    State (State&&) = delete;
    auto operator= (State const&) -> State& = delete;
    auto operator= (State&&) -> State& = delete;

  private:
    mcs::iov_backend::Parameter _parameter;

    MCS_UTIL_MEMBER_AUTO
      ( _io_context_client
      , mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u} // \todo parameter
        , SIGINT, SIGTERM
        }
      );
    template<mcs::util::ASIO::is_protocol Protocol>
      using Client
        = mcs::iov_backend::Client
          < Protocol
          , mcs::rpc::access_policy::Sequential
          >
        ;
    MCS_UTIL_MEMBER_AUTO
      ( _client
      , mcs::util::ASIO::run
        ( _parameter._provider
        , []<mcs::util::ASIO::is_protocol Protocol>
            ( mcs::util::ASIO::Connectable<Protocol> provider_connectable
            , decltype (_io_context_client)& io_context_client
            )
          {
            return std::variant< Client<asio::ip::tcp>
                               , Client<asio::local::stream_protocol>
                               >
              { std::in_place_type<Client<Protocol>>
              , io_context_client
              , provider_connectable
              };
          }
        , _io_context_client
        )
      );

    MCS_UTIL_MEMBER_AUTO
      ( _storages
      , mcs::core::Storages
          < mcs::iov_backend::SupportedStorageImplementations
          >{}
      );
    MCS_UTIL_MEMBER_AUTO
      ( _communication_buffer
      , Buffer<mcs::core::storage::implementation::Heap>
          { _storages
          ,   _parameter._indirect_communication._number_of_buffers
            * _parameter._indirect_communication._maximum_transfer_size
          }
      );
    MCS_UTIL_MEMBER_AUTO
      ( _bitmap
      , mcs::iov_backend::detail::Bitmap
        { _parameter._indirect_communication._number_of_buffers
        }
      );
    MCS_UTIL_MEMBER_AUTO
      ( _bitmap_interruption_context
      , mcs::iov_backend::detail::Bitmap::InterruptionContext{}
      );

    auto acquire_communication_buffer() -> mcs::iov_backend::AcquiredBuffer;

    struct SpanCompare
    {
      using is_transparent = std::true_type;

      // used to order the set by the address of the span
      [[nodiscard]] constexpr auto operator()
        ( std::span<std::byte> const& lhs
        , std::span<std::byte> const& rhs
        ) const noexcept -> bool
        ;

      // used to lower_bound
      [[nodiscard]] constexpr auto operator()
        ( std::span<std::byte> const& span
        , std::byte* address
        ) const noexcept -> bool
        ;
      [[nodiscard]] constexpr auto operator()
        ( std::span<std::byte const> const& span
        , std::byte const* address
        ) const noexcept -> bool
        ;
    };

    using SharedBufferImplementation = mcs::core::storage::implementation::Heap;
    MCS_UTIL_MEMBER_AUTO
      ( mutable _shared_buffer_guard
      , std::shared_mutex{}
      );
    MCS_UTIL_MEMBER_AUTO
      ( _shared_buffer
      , std::map< std::span<std::byte>
                , Buffer<SharedBufferImplementation>
                , SpanCompare
                >{}
      );

    MCS_UTIL_MEMBER_AUTO
      ( _io_context_transport_client
      , mcs::rpc::ScopedRunningIOContext
        { _parameter._number_of_threads._transport_clients
        , SIGINT, SIGTERM
        }
      );
    // \todo access_policy::Concurrent leads to logic_error "Unknown call_id"
    using TransportClientsAccessPolicy = mcs::rpc::access_policy::Sequential;
    struct HitMiss
    {
      using HitMissTraits = mcs::core::transport::implementation::ASIO::MapWithHitMissCallbacksTraits
        < TransportClientsAccessPolicy
        >;
      using Key = typename HitMissTraits::Key;
      using Value = typename HitMissTraits::Value;
      using Map = typename HitMissTraits::Map;

      // \todo eviction policy
      auto operator() (Key const&, Map const&) const noexcept -> void {}
      auto operator() (Key const&, Map&) const noexcept -> void {}
    };
    MCS_UTIL_MEMBER_AUTO
      ( _hit_miss
      , HitMiss{}
      );
    MCS_UTIL_MEMBER_AUTO
      ( _transport_client
      , mcs::core::transport::implementation::ASIO::make_clients_cache<TransportClientsAccessPolicy>
         ( _io_context_transport_client
         , std::addressof (_storages)
         , _hit_miss
         , _hit_miss
         )
      );

    // \todo is is required to protect from concurrent access?
    std::mutex _collection_create_delete_guard;

    struct ContainingBuffer
    {
      ContainingBuffer
        ( Buffer<SharedBufferImplementation> const&
        , mcs::core::memory::Size shift
        );

      [[nodiscard]] auto address
        ( mcs::core::memory::Offset
        ) const -> mcs::core::transport::Address
        ;

    private:
      Buffer<SharedBufferImplementation> const& _buffer;
      mcs::core::memory::Size _shift;
    };

    template<typename Span>
      [[nodiscard]] auto containing_shared_buffer (Span) const
        -> std::optional<ContainingBuffer>
      ;

    template<typename ReadOrWriteParameter>
      [[nodiscard]] auto locations
        ( ReadOrWriteParameter const&
        ) const -> mcs::iov_backend::Locations
      ;

    [[nodiscard]] auto write_direct
      ( iov::WriteParam
      , ContainingBuffer const&
      ) -> iov::Request<iov::WriteRequestData>
      ;
    [[nodiscard]] auto write_indirect
      ( iov::WriteParam
      ) -> iov::Request<iov::WriteRequestData>
      ;

    [[nodiscard]] auto read_direct
      ( iov::ReadParam
      , ContainingBuffer const&
      ) -> iov::Request<iov::ReadRequestData>
      ;
    [[nodiscard]] auto read_indirect
      ( iov::ReadParam
      ) -> iov::Request<iov::ReadRequestData>
      ;

    auto assert_range_is_inside
      ( mcs::iov_backend::collection::ID
      , off_t offset
      , std::size_t size
      ) const -> void
      ;

    auto ensure_range_is_inside_and_maybe_grow_the_collection
      ( mcs::iov_backend::collection::ID
      , mcs::util::not_null<iov::WriteParam>
      ) -> void
      ;
    auto collection_append
      ( mcs::iov_backend::collection::ID
      , mcs::util::not_null<iov::WriteParam>
      , mcs::core::memory::Range range_to_append
      ) -> void
      ;
  };

  static_assert (!std::is_copy_constructible_v<State>);
  static_assert (!std::is_move_constructible_v<State>);
  static_assert (!std::is_copy_assignable_v<State>);
  static_assert (!std::is_move_assignable_v<State>);
}

namespace
{
  auto backend_init
    ( std::filesystem::path config
    ) -> iov::BackendOps::State
  {
    return new State
      { mcs::util::read::from_file<mcs::iov_backend::Parameter> (config)
      };
  }

  auto backend_destroy (iov::BackendOps::State state) -> void
  {
    std::default_delete<State>{} (static_cast<State*> (state));
  }

  auto backend_read
    ( iov::BackendOps::State state
    , iov::ReadParam read_parameter
    ) noexcept -> iov::Request<iov::ReadRequestData>
  {
    return static_cast<State*> (state)->read (read_parameter);
  }

  auto backend_write
    ( iov::BackendOps::State state
    , iov::WriteParam write_parameter
    ) noexcept -> iov::Request<iov::WriteRequestData>
  {
    return static_cast<State*> (state)->write (write_parameter);
  }

  auto backend_allocate
      ( iov::BackendOps::State state
      , std::size_t size
      ) -> std::span<std::byte>
  {
    return static_cast<State*> (state)->allocate (size);
  }

  auto backend_free
      ( iov::BackendOps::State state
      , std::span<std::byte> span
      ) -> void
  {
    return static_cast<State*> (state)->free (span);
  }

  auto backend_collection_open
    ( iov::BackendOps::State state
    , iov::CollectionOpenParam meta_data_collection_open_parameter
    ) noexcept -> iov::expected<void, iov::ErrorReason>
  {
    return static_cast<State*> (state)->collection_open
      ( meta_data_collection_open_parameter
      );
  }

  auto backend_collection_close
    ( iov::BackendOps::State state
    , iov::CollectionCloseParam meta_data_collection_close_parameter
    ) noexcept -> iov::expected<void, iov::ErrorReason>
  {
    return static_cast<State*> (state)->collection_close
      ( meta_data_collection_close_parameter
      );
  }

  auto backend_collection_delete
    ( iov::BackendOps::State state
    , iov::DeleteParam meta_data_collection_delete_parameter
    ) noexcept -> iov::Request<iov::DeleteRequestData>
  {
    return static_cast<State*> (state)->collection_delete
      ( meta_data_collection_delete_parameter
      );
  }
}

namespace
{
  State::State (mcs::iov_backend::Parameter parameter)
    : _parameter {parameter}
  {}

  State::~State()
  {
    mcs::util::execute_and_die_on_exception
      ( "mcs::iov_backend::State::~State: bitmap.interrupt"
      , [&]
        {
          _bitmap.interrupt (_bitmap_interruption_context);
        }
      );
  }
}

namespace
{
  auto State::allocate (std::size_t size) -> std::span<std::byte>
  {
    auto buffer
      { Buffer<SharedBufferImplementation>
          { _storages
          , mcs::core::memory::make_size (size)
          }
      };
    auto span
      { buffer.template data<mcs::core::chunk::access::Mutable>
          ( mcs::core::memory::make_range
              ( mcs::core::memory::make_offset (0)
              , mcs::core::memory::make_size (size)
              )
          )
      };

    auto const lock {std::unique_lock {_shared_buffer_guard}};

    return _shared_buffer.emplace (span, std::move (buffer)).first->first;
  }

  auto State::free (std::span<std::byte> span) -> void
  {
    auto const lock {std::unique_lock {_shared_buffer_guard}};

    auto shared_buffer {_shared_buffer.find (span)};

    if (shared_buffer == std::end (_shared_buffer))
    {
      // \todo specific exception
      throw std::runtime_error {"State::free: Double free or corruption."};
    }

    _shared_buffer.erase (shared_buffer);
  }
}

namespace
{
  auto make_meta_data_key (mcs::iov_backend::collection::ID collection_id)
  {
    return iov::meta::Key {fmt::format ("mcs_iov_backend_{}", collection_id)};
  }
}

namespace
{
  // Stored in the iov meta data
  struct CollectionInformation
  {
    std::optional<std::size_t> size_max;
  };
  static_assert (mcs::serialization::is_serializable<CollectionInformation>);
}

namespace
{
  auto State::assert_range_is_inside
    ( mcs::iov_backend::collection::ID collection_id
    , off_t offset
    , std::size_t size
    ) const -> void
  {
    auto const range
      { mcs::core::memory::make_range
          ( mcs::core::memory::make_offset (offset)
          , mcs::core::memory::make_size (size)
          )
      };
    auto const memory_range
      { std::visit
          ( [&] (auto const& client)
            {
              return client.range (collection_id);
            }
          , _client
          )
      };

    if (  begin (range) < begin (memory_range)
       || end (memory_range) < end (range)
       )
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
            ( "mcs::iov_backend::read: "
              "range {} is not inside the memory range {}"
            , range
            , memory_range
            )
        };
    }
  }

  auto State::ensure_range_is_inside_and_maybe_grow_the_collection
    ( mcs::iov_backend::collection::ID collection_id
    , mcs::util::not_null<iov::WriteParam> write_parameter
    ) -> void
  {
    auto const range_to_write
      { mcs::core::memory::make_range
          ( mcs::core::memory::make_offset (write_parameter->offset)
          , mcs::core::memory::make_size (write_parameter->data.size())
          )
      };
    auto const collection_range
      { std::visit
          ( [&] (auto const& client)
            {
              return client.range (collection_id);
            }
          , _client
          )
      };

    if (begin (range_to_write) < begin (collection_range))
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
            ( "mcs::iov_backend::write: "
              "range {} is before the collection range {}"
            , range_to_write
            , collection_range
            )
        };
    }

    if (end (collection_range) < end (range_to_write))
    {
      collection_append (collection_id, write_parameter, range_to_write);
    }
  }

  auto State::collection_append
    ( mcs::iov_backend::collection::ID collection_id
    , mcs::util::not_null<iov::WriteParam> write_parameter
    , mcs::core::memory::Range range_to_append
    ) -> void
  {
    auto const meta_data_key {make_meta_data_key (collection_id)};

    auto const lock {std::unique_lock {_collection_create_delete_guard}};

    auto const serialized_collection_information
      { write_parameter->collection->metadata().get (meta_data_key)
      };

    if (!serialized_collection_information)
    {
      // \todo specific exception
      throw std::logic_error
        { fmt::format
          ( "mcs_iov_backend: write: collection '{}' does not exist."
          , collection_id
          )
        };
    }

    auto const collection_information
      { mcs::serialization::load_from<CollectionInformation>
        ( std::as_bytes
          ( std::span { std::begin (serialized_collection_information->str())
                      , std::end (serialized_collection_information->str())
                      }
          )
        )
      };

    if (collection_information.size_max.has_value())
    {
      // To grow beyond the end of the collection is forbidden if
      // there is a maximum size given. Note: Memory for max size
      // bytes is allocated up front during creation.

      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
          ( "mcs_iov_backend: write after max size '{}' of collection '{}'"
          , collection_information.size_max.value()
          , collection_id
          )
        };
    }

    auto const size
      { std::visit
        ( [&] (auto& client)
          {
            return client.collection_append (collection_id, range_to_append);
          }
        , _client
        )
      };

    mcs::iov_backend::invoke_and_throw_on_unexpected
      ( &iov::meta::MetaData::put
      , fmt::format ("Could not put '{}'", "size")
      , std::addressof (write_parameter->collection->metadata())
      , iov::meta::Key {"size"}
      , iov::meta::Value {std::to_string (size_cast<std::size_t> (size))}
      );
  }
}

namespace
{
  template<iov::RequestData RD>
    struct SynchronousRequest : public iov::RequestImplementation<RD>
  {
    SynchronousRequest (RD rd)
      : _rd {rd}
    {}

    [[nodiscard]] auto get() -> RD& override
    {
      return _rd;
    }
    auto wait() -> void override
    {
      return;
    }
    [[nodiscard]] auto test() -> iov::RequestStatus override
    {
      return iov::request::status::Ready{};
    }

  private:
    RD _rd;
  };

  template<iov::RequestData RD, typename... Args>
    requires (std::is_constructible_v<RD, Args...>)
    [[nodiscard]] auto make_synchronous_request
      ( Args&&... args
      ) -> iov::Request<RD>
  {
    return iov::Request<RD>
      { new SynchronousRequest<RD> {RD {std::forward<Args> (args)...}}
      , [] (auto* implementation) noexcept
        {
          std::default_delete<SynchronousRequest<RD>>{}
            (static_cast<SynchronousRequest<RD>*> (implementation));
        }
      };
  }
}

namespace
{
  template<iov::RequestData RD>
    struct ASynchronousRequest : public iov::RequestImplementation<RD>
  {
    ASynchronousRequest
      ( decltype (std::declval<RD>().params) params
      , std::vector<std::future<mcs::core::memory::Size>> operations
      )
        : _params {params}
        , _operations {std::move (operations)}
    {}

    [[nodiscard]] auto get() -> RD& override
    {
      if (!_rd)
      {
        try
        {
          _rd = RD
            { _params
            , size_cast<std::size_t>
                ( std::accumulate
                  ( std::begin (_operations), std::end (_operations)
                  , mcs::core::memory::make_size (0)
                  , [] (auto sum, auto& operation)
                    {
                      return sum + operation.get();
                    }
                  )
                )
            };
        }
        catch (...)
        {
          _rd = RD
            { _params
            , iov::unexpected
              ( iov::ErrorReason {fmt::format ("{}", std::current_exception())}
              )
            };
        }
      }

      return *_rd;
    }
    auto wait() -> void override
    {
      std::ranges::for_each
        ( _operations
        , [] (auto& operation)
          {
            operation.wait();
          }
        );
    }
    [[nodiscard]] auto test() -> iov::RequestStatus override
    {
      if ( std::any_of
           ( std::begin (_operations), std::end (_operations)
           , [] (auto& operation)
             {
               return operation.wait_for (std::chrono::microseconds (0))
                 == std::future_status::timeout
                 ;
             }
           )
         )
      {
        return iov::request::status::InProgress{};
      }

      return iov::request::status::Ready{};
    }

  private:
    decltype (std::declval<RD>().params) _params;
    std::vector<std::future<mcs::core::memory::Size>> _operations;
    std::optional<RD> _rd;
  };

  template<iov::RequestData RD, typename... Args>
    [[nodiscard]] auto make_asynchronous_request
      ( decltype (std::declval<RD>().params) params
      , std::vector<std::future<mcs::core::memory::Size>> operations
      ) -> iov::Request<RD>
  {
    return iov::Request<RD>
      { new ASynchronousRequest<RD> {params, std::move (operations)}
      , [] (auto* implementation) noexcept
        {
          std::default_delete<ASynchronousRequest<RD>>{}
            (static_cast<ASynchronousRequest<RD>*> (implementation));
        }
      };
  }
}

namespace
{
  auto State::acquire_communication_buffer
    (
    ) -> mcs::iov_backend::AcquiredBuffer
  {
    return mcs::iov_backend::AcquiredBuffer
      { std::addressof (_bitmap)
      , _bitmap_interruption_context
      , _parameter._indirect_communication
      };
  }
}

namespace
{
  auto make_collection_id
    ( iov::meta::MetaData const& meta_data
    ) -> mcs::iov_backend::collection::ID
  {
    return mcs::iov_backend::collection::ID {meta_data.uuid().str()};
  }

  template<typename Description, typename ReadOrWriteParameter>
    auto make_collection_id
      ( Description&& description
      , ReadOrWriteParameter const& parameter
      ) -> mcs::iov_backend::collection::ID
  {
    if (!parameter.collection)
    {
      throw std::logic_error
        { fmt::format ( "{}: collection ptr is null"
                      , std::forward<Description> (description)
                      )
        };
    }

    return make_collection_id (parameter.collection->metadata());
  }
}

namespace
{
  template<typename ReadOrWriteParameter>
    auto State::locations
      ( ReadOrWriteParameter const& parameter
      ) const -> mcs::iov_backend::Locations
  {
    return std::visit
      ( [&] (auto const& client)
        {
          return client.locations
            ( make_collection_id ("locations", parameter)
            , mcs::core::memory::make_range
              ( mcs::core::memory::make_offset (parameter.offset)
              , mcs::core::memory::make_size (parameter.data.size())
              )
            );
        }
      , _client
      );
  }
}

namespace
{
  struct Transfers
  {
    struct Transfer
    {
      mcs::core::transport::Address _address;
      mcs::core::memory::Size _size;
    };

    struct ConstIterator
    {
      using difference_type = std::ptrdiff_t;
      using value_type = Transfer;

      auto operator*() const noexcept -> value_type;
      auto operator++() noexcept -> ConstIterator&;
      auto operator++ (int) noexcept -> ConstIterator;

      auto operator!= (ConstIterator const&) const noexcept -> bool;
      auto operator== (ConstIterator const&) const noexcept -> bool;

      ConstIterator() noexcept = default;

    private:
      friend struct Transfers;
      ConstIterator ( mcs::core::transport::Address*
                    , mcs::core::memory::Size maximum_transfer_size
                    , mcs::core::memory::Size bytes_to_transfer
                    ) noexcept;

      mcs::core::transport::Address* _address {nullptr};
      mcs::core::memory::Size _maximum_transfer_size
        { mcs::core::memory::make_size (0)
        };
      mcs::core::memory::Size _bytes_to_transfer
        { mcs::core::memory::make_size (0)
        };
      mcs::core::memory::Size _transfer_size
        { std::min (_maximum_transfer_size, _bytes_to_transfer)
        };
    };
    static_assert (std::forward_iterator<ConstIterator>);

    auto begin() noexcept -> ConstIterator;
    auto end() noexcept -> ConstIterator;

    Transfers ( mcs::core::transport::Address
              , mcs::core::memory::Range
              , mcs::core::memory::Size
              ) noexcept;

  private:
    mcs::core::transport::Address _address;
    mcs::core::memory::Range _range;
    mcs::core::memory::Size _maximum_transfer_size;
  };
}

namespace
{
  Transfers::ConstIterator::ConstIterator
    ( mcs::core::transport::Address* address
    , mcs::core::memory::Size maximum_transfer_size
    , mcs::core::memory::Size bytes_to_transfer
    ) noexcept
      : _address {address}
      , _maximum_transfer_size {maximum_transfer_size}
      , _bytes_to_transfer {bytes_to_transfer}
  {}

  auto Transfers::ConstIterator::operator*() const noexcept -> value_type
  {
    return {*_address, _transfer_size};
  }

  auto Transfers::ConstIterator::operator++() noexcept -> ConstIterator&
  {
    _address->offset += _transfer_size;
    _transfer_size = std::min ( _maximum_transfer_size
                              , _bytes_to_transfer -= _transfer_size
                              );

    return *this;
  }

  auto Transfers::ConstIterator::operator++ (int) noexcept -> ConstIterator
  {
    auto state {*this};
    ++*this;
    return state;
  }

  auto Transfers::ConstIterator::operator!=
    ( ConstIterator const& other
    ) const noexcept -> bool
  {
    return _bytes_to_transfer != other._bytes_to_transfer;
  }

  auto Transfers::ConstIterator::operator==
    ( ConstIterator const& other
    ) const noexcept -> bool
  {
    return _bytes_to_transfer == other._bytes_to_transfer;
  }
}

namespace
{
  Transfers::Transfers ( mcs::core::transport::Address address
                       , mcs::core::memory::Range range
                       , mcs::core::memory::Size maximum_transfer_size
                       ) noexcept
    : _address {address}
    , _range {range}
    , _maximum_transfer_size {maximum_transfer_size}
  {}

  auto Transfers::begin() noexcept -> ConstIterator
  {
    return { std::addressof (_address)
           , _maximum_transfer_size
           , mcs::core::memory::size (_range)
           };
  }
  auto Transfers::end() noexcept -> ConstIterator
  {
    return {};
  }
}

namespace
{
  auto State::read_direct
    ( iov::ReadParam read_parameter
    , ContainingBuffer const& containing_buffer
    ) -> iov::Request<iov::ReadRequestData>
  {
    auto data_offset {mcs::core::memory::make_offset (0)};
    auto gets {std::vector<std::future<mcs::core::memory::Size>>{}};

    // \todo implementation does not effectively limit the number of
    // streams: the token is released directly after the stream has
    // been created. instead it must be released only after the stream
    // has been completed.
    auto streams
      { std::counting_semaphore
        { mcs::util::cast<std::ptrdiff_t>
          ( _parameter
            . _direct_communication
            . _maximum_number_of_parallel_streams
          )
        }
      };

    for (auto location : locations (read_parameter))
    {
      streams.acquire();

      std::visit
        ( [&] (auto&& client)
          {
            auto const release_stream
              { mcs::nonstd::make_scope_exit_that_dies_on_exception
                ( "iov_backend::read_direct::stream_release"
                , [&]() noexcept
                  {
                    streams.release();
                  }
                )
              };

            // \todo max ongoing transfer per stream
            for ( auto transfer
                : Transfers { location._address
                            , location._range
                            , _parameter
                              . _direct_communication
                              . _maximum_transfer_size
                            }
                )
            {
              gets.emplace_back
                ( client.memory_get
                    ( containing_buffer.address (data_offset)
                    , transfer._address
                    , transfer._size
                    )
                );

              data_offset += transfer._size;
            }
          }
        , _transport_client (location._transport_provider)
        );
    }

    return make_asynchronous_request<iov::ReadRequestData>
      ( read_parameter
      , std::move (gets)
      );
  }
}

namespace
{
  auto State::read_indirect
    ( iov::ReadParam read_parameter
    ) -> iov::Request<iov::ReadRequestData>
  {
    auto data_offset {mcs::core::memory::make_offset (0)};
    auto gets {std::vector<std::future<mcs::core::memory::Size>>{}};

    auto streams
      { std::make_shared<std::counting_semaphore<1>>
        ( mcs::util::cast<std::ptrdiff_t>
          ( _parameter
            . _indirect_communication
            . _maximum_number_of_parallel_streams
          )
        )
      };

    for (auto location : locations (read_parameter))
    {
      streams->acquire();

      gets.emplace_back
        ( std::async
          ( std::launch::async
          , [ location
            , data_offset
            , data = read_parameter.data.data()
            , streams
            , this
            ]
            {
              auto const release_stream
                { mcs::nonstd::make_scope_exit_that_dies_on_exception
                  ( "iov_backend::read_indirect::stream_release"
                  , [&]() noexcept
                    {
                      streams->release();
                    }
                  )
                };

              return std::visit
                ( [&] (auto&& client)
                  {
                    auto const communication_buffer
                      {acquire_communication_buffer()};

                    auto transfer_offset {data_offset};
                    auto bytes_processed {mcs::core::memory::make_size (0)};

                    for ( auto transfer
                        : Transfers { location._address
                                    , location._range
                                    , _parameter
                                      . _indirect_communication
                                      . _maximum_transfer_size
                                    }
                        )
                    {
                      client.memory_get
                        ( _communication_buffer.address
                            (communication_buffer.offset())
                        , transfer._address
                        , transfer._size
                        ).get();

                      std::copy_n
                        ( _communication_buffer
                          . template data<mcs::core::chunk::access::Const>
                            ( mcs::core::memory::make_range
                                ( communication_buffer.offset()
                                , transfer._size
                                )
                            ).data()
                        , size_cast<std::size_t> (transfer._size)
                        , data + transfer_offset
                        );

                      transfer_offset += transfer._size;
                      bytes_processed += transfer._size;
                    }

                    return bytes_processed;
                  }
                , _transport_client (location._transport_provider)
                );
            }
          )
        );

      data_offset += mcs::core::memory::size (location._range);
    }

    return make_asynchronous_request<iov::ReadRequestData>
      ( read_parameter
      , std::move (gets)
      );
  }
}

namespace
{
  auto State::read
    ( iov::ReadParam read_parameter
    ) noexcept -> iov::Request<iov::ReadRequestData>
  try
  {
    assert_range_is_inside
      ( make_collection_id ("read", read_parameter)
      , read_parameter.offset
      , read_parameter.data.size()
      );

    if ( auto const containing_buffer
           { containing_shared_buffer (read_parameter.data)
           }
       )
    {
      return read_direct (read_parameter, *containing_buffer);
    }

    return read_indirect (read_parameter);
  }
  catch (...)
  {
    return make_synchronous_request<iov::ReadRequestData>
      ( read_parameter
      , iov::unexpected
          ( iov::ErrorReason {fmt::format ("{}", std::current_exception())}
          )
      );
  }
}

namespace
{
  auto State::write_direct
    ( iov::WriteParam write_parameter
    , ContainingBuffer const& containing_buffer
    ) -> iov::Request<iov::WriteRequestData>
  {
    auto data_offset {mcs::core::memory::make_offset (0)};
    auto puts {std::vector<std::future<mcs::core::memory::Size>>{}};

    // \todo implementation does not effectively limit the number of
    // streams: the token is released directly after the stream has
    // been created. instead it must be released only after the stream
    // has been completed.
    auto streams
      { std::counting_semaphore
        { mcs::util::cast<std::ptrdiff_t>
          ( _parameter
            . _direct_communication
            . _maximum_number_of_parallel_streams
          )
        }
      };

    for (auto location : locations (write_parameter))
    {
      streams.acquire();

      std::visit
        ( [&] (auto&& client)
          {
            auto const release_stream
              { mcs::nonstd::make_scope_exit_that_dies_on_exception
                ( "iov_backend::write_direct::stream_release"
                , [&]() noexcept
                  {
                    streams.release();
                  }
                )
              };

            // \todo max ongoing transfer per stream
            for ( auto transfer
                : Transfers { location._address
                            , location._range
                            , _parameter
                              . _direct_communication
                              . _maximum_transfer_size
                            }
                )
            {
              puts.emplace_back
                ( client.memory_put
                    ( transfer._address
                    , containing_buffer.address (data_offset)
                    , transfer._size
                    )
                );

              data_offset += transfer._size;
            }
          }
        , _transport_client (location._transport_provider)
        );
    }

    return make_asynchronous_request<iov::WriteRequestData>
      ( write_parameter
      , std::move (puts)
      );
  }
}

namespace
{
  auto State::write_indirect
    ( iov::WriteParam write_parameter
    ) -> iov::Request<iov::WriteRequestData>
  {
    auto data_offset {mcs::core::memory::make_offset (0)};
    auto puts {std::vector<std::future<mcs::core::memory::Size>>{}};

    auto streams
      { std::make_shared<std::counting_semaphore<1>>
        ( mcs::util::cast<std::ptrdiff_t>
          ( _parameter
            . _indirect_communication
            . _maximum_number_of_parallel_streams
          )
        )
      };

    for (auto location : locations (write_parameter))
    {
      streams->acquire();

      puts.emplace_back
        ( std::async
          ( std::launch::async
          , [ location
            , data_offset
            , data = write_parameter.data.data()
            , streams
            , this
            ]
            {
              auto const release_stream
                { mcs::nonstd::make_scope_exit_that_dies_on_exception
                  ( "iov_backend::write_indirect::stream_release"
                  , [&]() noexcept
                    {
                      streams->release();
                    }
                  )
                };

              return std::visit
                ( [&] (auto&& client)
                  {
                    auto const communication_buffer
                      {acquire_communication_buffer()};

                    auto transfer_offset {data_offset};
                    auto bytes_processed {mcs::core::memory::make_size (0)};

                    for ( auto transfer
                        : Transfers { location._address
                                    , location._range
                                    , _parameter
                                      . _indirect_communication
                                      . _maximum_transfer_size
                                    }
                        )
                    {
                      std::copy_n
                        ( data + transfer_offset
                        , size_cast<std::size_t> (transfer._size)
                        , _communication_buffer
                          . template data<mcs::core::chunk::access::Mutable>
                            ( mcs::core::memory::make_range
                                ( communication_buffer.offset()
                                , transfer._size
                                )
                            ).data()
                        );

                      client.memory_put
                        ( transfer._address
                        , _communication_buffer.address
                            (communication_buffer.offset())
                        , transfer._size
                        ).get();

                      transfer_offset += transfer._size;
                      bytes_processed += transfer._size;
                    }

                    return bytes_processed;
                  }
                , _transport_client (location._transport_provider)
                );
            }
          )
        );

      data_offset += mcs::core::memory::size (location._range);
    }

    return make_asynchronous_request<iov::WriteRequestData>
      ( write_parameter
      , std::move (puts)
      );
  }
}

namespace
{
  auto State::write
    ( iov::WriteParam write_parameter
    ) noexcept -> iov::Request<iov::WriteRequestData>
  try
  {
    ensure_range_is_inside_and_maybe_grow_the_collection
      ( make_collection_id ("write", write_parameter)
      , std::addressof (write_parameter)
      );

    if ( auto const containing_buffer
           { containing_shared_buffer (write_parameter.data)
           }
       )
    {
      return write_direct (write_parameter, *containing_buffer);
    }

    return write_indirect (write_parameter);
  }
  catch (...)
  {
    return make_synchronous_request<iov::WriteRequestData>
      ( write_parameter
      , iov::unexpected
        ( iov::ErrorReason {fmt::format ("{}", std::current_exception())}
        )
      );
  }
}

namespace
{
  auto State::collection_open
    ( iov::CollectionOpenParam meta_data_collection_open_parameter
    ) noexcept -> iov::expected<void, iov::ErrorReason>
  try
  {
    if (!meta_data_collection_open_parameter.meta_data)
    {
      throw std::invalid_argument {"collection_open: meta data is nullptr"};
    }

    auto const maybe_size
      { [&] (auto control)
        {
          return std::visit
            ( mcs::util::overloaded
              { [&] ( iov::size::limit::Value value
                    ) noexcept -> std::optional<std::size_t>
                {
                  return value.value;
                }
              , [] ( iov::size::limit::Unknown
                   ) noexcept -> std::optional<std::size_t>
                {
                  return {};
                }
              }
            , control
            );
        }
      };

    auto const size_min
      { maybe_size
        ( meta_data_collection_open_parameter.controls
          .size().collection_size.min
        )
      };
    auto const size_max
      { maybe_size
        ( meta_data_collection_open_parameter.controls
          .size().collection_size.max
        )
      };

    auto const collection_id
      { make_collection_id (*meta_data_collection_open_parameter.meta_data)
      };
    auto const meta_data_key {make_meta_data_key (collection_id)};
    auto const collection_information
      { CollectionInformation
        { size_max
        }
      };
    auto const meta_data_value
      { std::ostringstream{}
        << mcs::serialization::OArchive {collection_information}
      };

    auto const lock {std::unique_lock {_collection_create_delete_guard}};

    if (!meta_data_collection_open_parameter.meta_data->get (meta_data_key))
    {
      auto const size
        { mcs::core::memory::make_size
          ( size_max.value_or (size_min.value_or (0UL))
          )
        };

      std::visit
        ( [&] (auto& client)
          {
            return client.collection_create (collection_id, size);
          }
        , _client
        );

      mcs::iov_backend::invoke_and_throw_on_unexpected
        ( &iov::meta::MetaData::put
        , fmt::format ("Could not put '{}'", meta_data_key)
        , meta_data_collection_open_parameter.meta_data
        , meta_data_key
        , iov::meta::Value {meta_data_value.str()}
        );

      mcs::iov_backend::invoke_and_throw_on_unexpected
        ( &iov::meta::MetaData::put
        , fmt::format ("Could not put '{}'", "size")
        , meta_data_collection_open_parameter.meta_data
        , iov::meta::Key {"size"}
        , iov::meta::Value {std::to_string (size_cast<std::size_t> (size))}
        );
    }

    return {};
  }
  catch (...)
  {
    return iov::unexpected
      { iov::ErrorReason
        { fmt::format
            ( "mcs_iov_backend: collection_open: {}"
            , std::current_exception()
            )
        }
      };
  }
}

namespace
{
  auto State::collection_close
    ( iov::CollectionCloseParam meta_data_collection_close_parameter
    ) noexcept -> iov::expected<void, iov::ErrorReason>
  {
    auto const collection_id
      { make_collection_id (*meta_data_collection_close_parameter.meta_data)
      };
    auto const meta_data_key {make_meta_data_key (collection_id)};

    if (!meta_data_collection_close_parameter.meta_data->get (meta_data_key))
    {
      return iov::unexpected
        { iov::ErrorReason
            { fmt::format
              ( "mcs_iov_backend: collection_close:"
                " collection '{}' does not exist."
              , collection_id
              )
            }
        };
    }

    return {};
  }
}

namespace
{
  auto State::collection_delete
    ( iov::DeleteParam meta_data_collection_delete_parameter
    ) noexcept -> iov::Request<iov::DeleteRequestData>
  try
  {
    if (!meta_data_collection_delete_parameter.meta_data)
    {
      throw std::invalid_argument {"collection_delete: meta data is nullptr"};
    }

    auto const collection_id
      { make_collection_id (*meta_data_collection_delete_parameter.meta_data)
      };
    auto const meta_data_key {make_meta_data_key (collection_id)};

    auto const lock {std::unique_lock {_collection_create_delete_guard}};

    if (meta_data_collection_delete_parameter.meta_data->get (meta_data_key))
    {
      std::visit
        ( [&] (auto& client)
          {
            return client.collection_delete (collection_id);
          }
        , _client
        );

      mcs::iov_backend::invoke_and_throw_on_unexpected
        ( &iov::meta::MetaData::remove
        , fmt::format ("Could not remove '{}'", meta_data_key)
        , meta_data_collection_delete_parameter.meta_data
        , meta_data_key
        );
    }

    return make_synchronous_request<iov::DeleteRequestData>
      ( meta_data_collection_delete_parameter
      , iov::expected<void, iov::ErrorReason>{}
      );
  }
  catch (...)
  {
    return make_synchronous_request<iov::DeleteRequestData>
      ( meta_data_collection_delete_parameter
      , iov::unexpected
        ( iov::ErrorReason
          { fmt::format
              ( "mcs_iov_backend: collection_delete: {}"
              , std::current_exception()
              )
          }
        )
      );
  }
}

namespace
{
  State::ContainingBuffer::ContainingBuffer
    ( Buffer<SharedBufferImplementation> const& buffer
    , mcs::core::memory::Size shift
    )
      : _buffer {buffer}
      , _shift {shift}
  {}

  auto State::ContainingBuffer::address
    ( mcs::core::memory::Offset offset
    ) const -> mcs::core::transport::Address
  {
    return _buffer.address (offset + _shift);
  }
}

namespace
{
  constexpr auto State::SpanCompare::operator()
    ( std::span<std::byte> const& lhs
    , std::span<std::byte> const& rhs
    ) const noexcept -> bool
  {
    return std::less{} (lhs.data(), rhs.data());
  }

  constexpr auto State::SpanCompare::operator()
    ( std::span<std::byte> const& span
    , std::byte* address
    ) const noexcept -> bool
  {
    return ! std::less{} (address, span.data() + span.size());
  }
  constexpr auto State::SpanCompare::operator()
    ( std::span<std::byte const> const& span
    , std::byte const* address
    ) const noexcept -> bool
  {
    return ! std::less{} (address, span.data() + span.size());
  }
}

namespace
{
  template<typename Span>
    auto State::containing_shared_buffer (Span span) const
      -> std::optional<ContainingBuffer>
  {
    auto const span_begin {span.data()};
    auto const span_end {span_begin + span.size()};

    auto shared_buffer
      { std::invoke
          ( [&]
            {
              auto const lock {std::shared_lock {_shared_buffer_guard}};

              return _shared_buffer.lower_bound (span_begin);
            }
          )
      };

    if (shared_buffer == std::end (_shared_buffer))
    {
      // The span starts after the last shared buffer.
      return {};
    }

    auto const& buffer {shared_buffer->first};
    auto const buffer_begin {buffer.data()};
    auto const buffer_end {buffer_begin + buffer.size()};

    // By the property of lower_bound shared_buffer is the smallest
    // buffer such that
    assert (span_begin < buffer_end);

    if (span_end < buffer_begin)
    {
      // The span ends before the first shared buffer.
      return {};
    }

    // Now excluded are the cases:
    // buffer               [---------)
    // span          [---)  :         :
    //                      :         [---)
    //                      :         :   [---)

    // Still included are the cases:
    // buffer               [---------)
    // span          [----------)     :        // span_begin < buffer.begin
    //                      : [---)   :
    //                      : [-----------)    // buffer_end < span.end

    // Check for partial overlap and throw if that is the case:
    // \todo is that even possible?
    if ((span_begin < buffer_begin) || (buffer_end < span_end))
    {
      // \todo specific exception
      throw std::runtime_error
        { fmt::format
            ( "mcs::iov_backend::Unsupported: The span [{},{}) has parts inside and parts outside of the shared buffer [{},{})."
            , static_cast<void const*> (span_begin)
            , static_cast<void const*> (span_end)
            , static_cast<void const*> (buffer_begin)
            , static_cast<void const*> (buffer_end)
            )
        };
    }

    return ContainingBuffer
      { shared_buffer->second
      , mcs::core::memory::make_size (buffer_begin - span_begin)
      };
  }
}

namespace
{
  template< mcs::iov_backend::is_supported_storage_implementation
              StorageImplementation
          >
    Buffer<StorageImplementation>::Buffer
      ( mcs::core::Storages
          < mcs::iov_backend::SupportedStorageImplementations
          >& storages
      , mcs::core::memory::Size size
      )
        : _storages {storages}
        , _size {size}
  {}

  template< mcs::iov_backend::is_supported_storage_implementation
              StorageImplementation
          >
    auto Buffer<StorageImplementation>::address
      ( mcs::core::memory::Offset offset
      ) const -> mcs::core::transport::Address
  {
    return mcs::core::transport::Address
      { _storage->id()
      , mcs::core::storage::make_parameter
          ( typename StorageImplementation::Parameter::Chunk::Description{}
          )
      , _segment->id()
      , offset
      };
  }

  template< mcs::iov_backend::is_supported_storage_implementation
              StorageImplementation
          >
    template<mcs::core::chunk::is_access Access>
      auto Buffer<StorageImplementation>::data (mcs::core::memory::Range range) const
        -> typename Access::template Span<std::byte>
  {
    return mcs::core::as<typename Access::template ValueType<std::byte>>
      ( mcs::iov_backend::SupportedStorageImplementations::template wrap
          < mcs::core::Chunk
          , Access
          >
            { _storages.template chunk_description
                  < StorageImplementation
                  , Access
                  >
              ( _storages.read_access()
              , _storage->id()
              , typename StorageImplementation::Parameter::Chunk::Description{}
              , _segment->id()
              , range
              )
            }
      );
  }
}

namespace mcs::iov_backend
{
  namespace
  {
    AcquiredBuffer::AcquiredBuffer
      ( detail::Bitmap* bitmap
      , detail::Bitmap::InterruptionContext const& bitmap_interruption_context
      , Parameter::IndirectCommunication const& indirect_communication
      )
        : _indirect_communication {indirect_communication}
        , _bit
          { decltype (_bit)
            { new detail::Bitmap::Index
              { std::invoke
                ( [&]
                  {
                    return bitmap->set
                      ( bitmap_interruption_context
                      , std::chrono::system_clock::now()
                        + std::chrono::milliseconds
                          ( indirect_communication
                          . _acquire_buffer_timeout_in_milliseconds
                          )
                      );
                    // \todo!? nested_exception
                  }
                )
              }
            , Deleter {bitmap}
            }
          }
      {}

    auto AcquiredBuffer::offset() const noexcept -> core::memory::Offset
    {
      return core::memory::make_offset (0)
        + *_bit * _indirect_communication._maximum_transfer_size
        ;
    }

    AcquiredBuffer::Deleter::Deleter (detail::Bitmap* bitmap) noexcept
      : _bitmap {bitmap}
    {}

    auto AcquiredBuffer::Deleter::operator()
      ( detail::Bitmap::Index* bit
      ) const noexcept -> void
    {
      _bitmap->clear (*bit);

      std::default_delete<detail::Bitmap::Index>{} (bit);
    }
  }
}
