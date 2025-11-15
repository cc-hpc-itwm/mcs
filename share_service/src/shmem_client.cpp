// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <concepts>
#include <cstdlib>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <mcs/core/Chunk.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Concurrent.hpp>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/Client.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/share_service/shmem_client.hpp>
#include <mcs/util/require_semi.hpp>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace mcs::share_service
{
#define MCS_SHARE_SERVICE_PIMPL(_prefix, _name)                         \
                                                                        \
  _prefix::_name::_name (Implementation* implementation) noexcept       \
    : _implementation {implementation}                                  \
  {}                                                                    \
                                                                        \
  _prefix::_name (_name&& other) noexcept                               \
    : _implementation (std::exchange (other._implementation, nullptr))  \
  {}                                                                    \
                                                                        \
  auto _prefix::operator= (_name&& other) noexcept -> _name&            \
  {                                                                     \
   _implementation = std::exchange (other._implementation, nullptr);    \
                                                                        \
   return *this;                                                        \
  }                                                                     \
                                                                        \
  _prefix::~_name() noexcept                                            \
  {                                                                     \
    delete _implementation;                                             \
  }                                                                     \
                                                                        \
  MCS_UTIL_REQUIRE_SEMI()

  // ----------------------------------------------------------------------

  template<typename T, typename... Args>
    auto new_noexcept (Args&&... args) noexcept -> T*
  {
    try
    {
      return new T {std::forward<Args> (args)...};
    }
    catch (...)
    {
      return nullptr;
    }
  }

  template<typename T>
    auto not_null (T* x) -> T*
  {
    if (!x)
    {
      std::abort();
    }

    return x;
  }
  template<typename T>
    auto not_null (std::optional<T> const& x) -> T const&
  {
    if (!x)
    {
      std::abort();
    }

    return *x;
  }

  // ----------------------------------------------------------------------

  struct shmem_client::NumberOfThreads::Implementation
  {
    std::uint32_t _value;
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client::NumberOfThreads, NumberOfThreads);

  shmem_client::NumberOfThreads::NumberOfThreads (std::uint32_t value) noexcept
    : NumberOfThreads {new_noexcept<Implementation> (value)}
  {}

  // ----------------------------------------------------------------------

  struct shmem_client::Size::Implementation
  {
    std::uint_least64_t _value;
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client::Size, Size);

  shmem_client::Size::Size (std::uint_least64_t value) noexcept
    : Size {new_noexcept<Implementation> (value)}
  {}

  // ----------------------------------------------------------------------

#define MCS_SHARE_SERVICE_IMPLEMENRATION_FROM_ERROR()   \
  Implementation (std::exception_ptr error) noexcept    \
    : _error {fmt::format ("{}", error)}                \
  {}
#define MCS_SHARE_SERVICE_IMPLEMENTATION_OK(_name)      \
  auto _name::ok() const noexcept -> bool               \
  {                                                     \
    return !_implementation                             \
      || !_implementation->_error.has_value();          \
  } MCS_UTIL_REQUIRE_SEMI()
#define MCS_SHARE_SERVICE_IMPLEMENTATION_ERROR(_name)   \
  auto _name::error() const noexcept -> char const*     \
  {                                                     \
    if (!_implementation)                               \
    {                                                   \
      return "implementation is null";                  \
    }                                                   \
                                                        \
    return not_null (_implementation->_error).c_str();  \
  } MCS_UTIL_REQUIRE_SEMI()

  namespace
  {
    template<typename Impl, typename... Args>
      requires
        ( std::is_constructible_v<Impl, Args...>
          and
          std::is_constructible_v<Impl, std::exception_ptr>
        )
      auto implementation_create (Args&&... args) noexcept -> Impl*
    try
    {
      return new_noexcept<Impl> (std::forward<Args> (args)...);
    }
    catch (...)
    {
      return new_noexcept<Impl> (std::current_exception());
    }

    template<typename Ret, typename Fun>
      auto create_from_chunk
        ( shmem_client::Chunk const& chunk
        , Fun&& fun
        ) noexcept
    try
    {
      if (!chunk.ok())
      {
        throw std::invalid_argument {"Chunk is not ok."};
      }

      return new_noexcept<typename Ret::Implementation>
        ( std::invoke (std::forward<Fun> (fun))
        );
    }
    catch (...)
    {
      return new_noexcept<typename Ret::Implementation>
        (std::current_exception());
    }
  }

  // ----------------------------------------------------------------------

  struct shmem_client::Chunk::Implementation
  {
    std::optional<std::string> _error{};
    std::optional<mcs::share_service::Chunk> _chunk{};
    std::optional<std::string> _data;

    Implementation (mcs::share_service::Chunk chunk) noexcept
      : _chunk {chunk}
      , _data {fmt::format ("{}", *_chunk)}
    {}
    MCS_SHARE_SERVICE_IMPLEMENRATION_FROM_ERROR()
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client::Chunk, Chunk);
  MCS_SHARE_SERVICE_IMPLEMENTATION_OK (shmem_client::Chunk);
  MCS_SHARE_SERVICE_IMPLEMENTATION_ERROR (shmem_client::Chunk);

  auto shmem_client::Chunk::text() const noexcept -> Text
  {
    return not_null (not_null (_implementation)->_data).c_str();
  }

  auto shmem_client::Chunk::text_size() const noexcept -> TextSize
  {
    auto const size {not_null (not_null (_implementation)->_data).size()};
    static_assert (sizeof (size) == sizeof (std::uint64_t));
    return size;
  }

  shmem_client::Chunk::Chunk
    ( Text text
    , TextSize text_size
    ) noexcept
      : Chunk
        { implementation_create<Implementation>
            ( util::read::read<mcs::share_service::Chunk> (text, text_size)
            )
        }
  {}

  // ----------------------------------------------------------------------

  struct shmem_client::MutableSpan::Implementation
  {
    using CoreChunk = mcs::share_service::SupportedStorageImplementations
      ::template wrap<mcs::core::Chunk, mcs::core::chunk::access::Mutable>
      ;

    std::optional<std::string> _error{};
    std::optional<CoreChunk> _chunk{};

    Implementation (CoreChunk chunk) noexcept
      : _chunk {std::move (chunk)}
    {}
    MCS_SHARE_SERVICE_IMPLEMENRATION_FROM_ERROR()
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client::MutableSpan, MutableSpan);

  MCS_SHARE_SERVICE_IMPLEMENTATION_OK (shmem_client::MutableSpan);
  MCS_SHARE_SERVICE_IMPLEMENTATION_ERROR (shmem_client::MutableSpan);

  auto shmem_client::MutableSpan::data() const noexcept -> Data
  {
    return not_null (not_null (_implementation)->_chunk).data().data();
  }
  auto shmem_client::MutableSpan::size() const noexcept -> DataSize
  {
    auto const size
      {not_null (not_null (_implementation)->_chunk).data().size()};
    static_assert (sizeof (size) == sizeof (std::uint64_t));
    return size;
  }

  // ----------------------------------------------------------------------

  struct shmem_client::ConstSpan::Implementation
  {
    using CoreChunk = mcs::share_service::SupportedStorageImplementations
      ::template wrap<mcs::core::Chunk, mcs::core::chunk::access::Const>
      ;

    std::optional<std::string> _error{};
    std::optional<CoreChunk> _chunk{};

    Implementation (CoreChunk chunk) noexcept
      : _chunk {std::move (chunk)}
    {}
    MCS_SHARE_SERVICE_IMPLEMENRATION_FROM_ERROR()
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client::ConstSpan, ConstSpan);

  MCS_SHARE_SERVICE_IMPLEMENTATION_OK (shmem_client::ConstSpan);
  MCS_SHARE_SERVICE_IMPLEMENTATION_ERROR (shmem_client::ConstSpan);

  auto shmem_client::ConstSpan::data() const noexcept -> Data
  {
    return not_null (not_null (_implementation)->_chunk).data().data();
  }
  auto shmem_client::ConstSpan::size() const noexcept -> DataSize
  {
    auto const size
      {not_null (not_null (_implementation)->_chunk).data().size()};
    static_assert (sizeof (size) == sizeof (std::uint64_t));
    return size;
  }

  // ----------------------------------------------------------------------

  struct shmem_client::Void::Implementation
  {
    std::optional<std::string> _error{};

    Implementation() noexcept = default;
    MCS_SHARE_SERVICE_IMPLEMENRATION_FROM_ERROR()
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client::Void, Void);

  MCS_SHARE_SERVICE_IMPLEMENTATION_OK (shmem_client::Void);
  MCS_SHARE_SERVICE_IMPLEMENTATION_ERROR (shmem_client::Void);

  // ----------------------------------------------------------------------

  struct shmem_client::Implementation
  {
    std::optional<std::string> _error{};
    std::optional<rpc::ScopedRunningIOContext> _io_context{};
    template<rpc::is_protocol Protocol>
      using Client = mcs::share_service::Client
                   < Protocol
                   , rpc::access_policy::Concurrent
                   >
      ;
    std::optional<std::variant< Client<asio::local::stream_protocol>
                              , Client<asio::ip::tcp>
                              >
                 > _client{};

    template<typename... Signals>
      Implementation ( std::uint32_t number_of_threads
                     , char const* socket
                     , Signals&&... signals
                     )
        : _io_context
          { std::in_place
          , rpc::ScopedRunningIOContext::NumberOfThreads {number_of_threads}
          , std::forward<Signals> (signals)...
          }
        , _client
          { std::in_place
          , Client<asio::local::stream_protocol>
            { *_io_context
            , util::ASIO::Connectable<asio::local::stream_protocol> {socket}
            }
          }
    {}
    template<typename Protocol, typename... Signals>
      Implementation ( Protocol protocol
                     , std::uint32_t number_of_threads
                     , std::uint16_t port
                     , Signals&&... signals
                     )
        : _io_context
          { std::in_place
          , rpc::ScopedRunningIOContext::NumberOfThreads {number_of_threads}
          , std::forward<Signals> (signals)...
          }
        , _client
          { std::in_place
          , Client<asio::ip::tcp>
            { *_io_context
            , util::ASIO::Connectable<asio::ip::tcp>
                {asio::ip::tcp::endpoint {protocol, port}}
            }
          }
    {}
    MCS_SHARE_SERVICE_IMPLEMENRATION_FROM_ERROR()
  };

  MCS_SHARE_SERVICE_PIMPL (shmem_client, shmem_client);

  MCS_SHARE_SERVICE_IMPLEMENTATION_OK (shmem_client);
  MCS_SHARE_SERVICE_IMPLEMENTATION_ERROR (shmem_client);

  auto shmem_client::create
    ( Size size
    , Prefix prefix
    , AccessMode access_mode
    , MLOCKed mlocked
    ) const noexcept -> Chunk
  {
    return implementation_create<Chunk::Implementation>
      ( std::visit
        ( [&] (auto const& client)
          {
            using SHMEM = mcs::core::storage::implementation::SHMEM;
            namespace Command = mcs::share_service::command;
            using MaybeMLOCKed
              = std::optional<SHMEM::Parameter::Segment::MLOCKed>
              ;

            return client.create
              ( mcs::core::memory::make_size
                  ( not_null (size._implementation)->_value
                  )
              , Command::create::Parameters<SHMEM>
                { SHMEM::Parameter::Create
                  { SHMEM::Prefix {util::string {prefix}}
                  , mcs::core::storage::MaxSize::Limit
                      { mcs::core::memory::make_size
                        (not_null (size._implementation)->_value)
                      }
                  }
                , SHMEM::Parameter::Segment::Create
                  { SHMEM::Parameter::Segment::AccessMode {access_mode}
                  , mlocked
                    ? MaybeMLOCKed {SHMEM::Parameter::Segment::MLOCKed{}}
                    : MaybeMLOCKed{}
                  }
                }
              );
          }
          , not_null (not_null (_implementation)->_client)
        )
      );
  }

  auto shmem_client::attach_mutable
    ( Chunk const& chunk
    ) const noexcept -> MutableSpan
  {
    return create_from_chunk<MutableSpan>
      ( chunk
      , [&]
        {
          return std::visit
            ( [&] (auto const& client)
              {
                using SHMEM = mcs::core::storage::implementation::SHMEM;
                namespace Command = mcs::share_service::command;
                using MutableAccess = mcs::core::chunk::access::Mutable;

                return client.template attach<MutableAccess>
                  ( not_null (not_null (chunk._implementation)->_chunk)
                  , Command::attach::Parameters<SHMEM>
                    { SHMEM::Parameter::Chunk::Description{}
                    }
                  );
              }
            , not_null (not_null (_implementation)->_client)
            );
        }
      );
  }

  auto shmem_client::attach_const
    ( Chunk const& chunk
    ) const noexcept -> ConstSpan
  {
    return create_from_chunk<ConstSpan>
      ( chunk
      , [&]
        {
          return std::visit
            ( [&] (auto const& client)
              {
                using SHMEM = mcs::core::storage::implementation::SHMEM;
                namespace Command = mcs::share_service::command;
                using ConstAccess = mcs::core::chunk::access::Const;

                return client.template attach<ConstAccess>
                  ( not_null (not_null (chunk._implementation)->_chunk)
                  , Command::attach::Parameters<SHMEM>
                    { SHMEM::Parameter::Chunk::Description{}
                    }
                  );
              }
            , not_null (not_null (_implementation)->_client)
            );
        }
      );
  }

  auto shmem_client::remove
    ( Chunk const& chunk
    ) const noexcept -> Void
  {
    return create_from_chunk<Void>
      ( chunk
      , [&]
        {
          return std::visit
            ( [&] (auto& client)
              {
                using SHMEM = mcs::core::storage::implementation::SHMEM;
                namespace Command = mcs::share_service::command;

                client.remove
                  ( not_null (not_null (chunk._implementation)->_chunk)
                  , Command::remove::Parameters<SHMEM>
                    { SHMEM::Parameter::Segment::Remove{}
                    }
                  );

                return Void::Implementation{};
              }
            , not_null (not_null (_implementation)->_client)
            );
        }
      );
  }

  auto make_shmem_local_stream_client
    ( shmem_client::NumberOfThreads number_of_threads
    , char const* socket
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( not_null (number_of_threads._implementation)->_value
      , socket
      );
  }
  auto make_shmem_local_stream_client
    ( shmem_client::NumberOfThreads number_of_threads
    , char const* socket
    , std::int32_t signal
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( not_null (number_of_threads._implementation)->_value
      , socket
      , signal
      );
  }
  auto make_shmem_local_stream_client
    ( shmem_client::NumberOfThreads number_of_threads
    , char const* socket
    , std::int32_t signal1
    , std::int32_t signal2
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( not_null (number_of_threads._implementation)->_value
      , socket
      , signal1
      , signal2
      );
  }
  auto make_shmem_local_stream_client
    ( shmem_client::NumberOfThreads number_of_threads
    , char const* socket
    , std::int32_t signal1
    , std::int32_t signal2
    , std::int32_t signal3
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( not_null (number_of_threads._implementation)->_value
      , socket
      , signal1
      , signal2
      , signal3
      );
  }

  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V4
    , std::uint16_t port
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v4()
      , not_null (number_of_threads._implementation)->_value
      , port
      );
  }
  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V4
    , std::uint16_t port
    , std::int32_t signal
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v4()
      , not_null (number_of_threads._implementation)->_value
      , port
      , signal
      );
  }
  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V4
    , std::uint16_t port
    , std::int32_t signal1
    , std::int32_t signal2
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v4()
      , not_null (number_of_threads._implementation)->_value
      , port
      , signal1
      , signal2
      );
  }
  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V4
    , std::uint16_t port
    , std::int32_t signal1
    , std::int32_t signal2
    , std::int32_t signal3
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v4()
      , not_null (number_of_threads._implementation)->_value
      , port
      , signal1
      , signal2
      , signal3
      );
  }

  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V6
    , std::uint16_t port
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v6()
      , not_null (number_of_threads._implementation)->_value
      , port
      );
  }
  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V6
    , std::uint16_t port
    , std::int32_t signal
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v6()
      , not_null (number_of_threads._implementation)->_value
      , port
      , signal
      );
  }
  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V6
    , std::uint16_t port
    , std::int32_t signal1
    , std::int32_t signal2
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v6()
      , not_null (number_of_threads._implementation)->_value
      , port
      , signal1
      , signal2
      );
  }
  auto make_shmem_tcp_client
    ( shmem_client::NumberOfThreads number_of_threads
    , TCPVersion::V6
    , std::uint16_t port
    , std::int32_t signal1
    , std::int32_t signal2
    , std::int32_t signal3
    ) noexcept -> shmem_client
  {
    return implementation_create<shmem_client::Implementation>
      ( asio::ip::tcp::v6()
      , not_null (number_of_threads._implementation)->_value
      , port
      , signal1
      , signal2
      , signal3
      );
  }
#undef MCS_SHARE_SERVICE_PIMPL
}
