// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#ifndef MCS_SHARE_SERVICE_SHMEM_CLIENT_HPP
#define MCS_SHARE_SERVICE_SHMEM_CLIENT_HPP

// C++11 interface with stable ABI to an mcs_share_service Client that
// uses the SHMEM memory backend.

// Note on error handling: All functions are 'noexcept' in order to
// guarantee ABI stability. All returned objects have two observers:
//
//   ok() const noexcept -> bool
//   error() const noexcept -> char const*
//
// Objects are only valid of 'ok() == true'. In this case to call
// 'error()' is undefined behavior. All other members functions are ok
// to call.
//
// If, however, 'ok() == false' then to call 'error()' returns a
// textual representation of the error that prevented the construction
// to succeed. To call any other member function is undefined
// behavior.
//
// To summarize:
//
// To call 'ok()' is always defined behavior. To call other functions
// might be undefined, depending on the return value of 'ok()':
//
//     return value of 'ok()'  call 'error()'  call other member function
//     ----------------------  --------------  --------------------------
//       true                    undefined       ok
//       false                   ok              undefined
//
// EXAMPLE:
//
//     auto const mspan {client.attach_mutable (chunk)};
//     if (!mspan.ok())
//     {
//       throw std::runtime_error {mspan.error()};
//     }
//     auto span {std::span { mcs::util::cast<int*> (mspan.data())
//                          , mspan.size() / sizeof (int)
//                          }
//               };

// For examples see test/mcs/share_service/shmem_client.cpp

#include <cstdint>

#if (__cplusplus >= 201703L)
# define MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD [[nodiscard]]
#else
# define MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD
#endif

#define MCS_SHARE_SERVICE_SHMEM_CLIENT_CONSTRUCTOR(_name, _arg...)      \
  explicit _name (_arg) noexcept

#define MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER(_name)    \
  MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD                      \
  auto _name() const noexcept

#define MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_MEMBER(_name, _param...) \
  MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD                            \
  auto _name (_param) const noexcept

#define MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL(_name)     \
                                                        \
  _name (_name const&) = delete;                        \
  auto operator= (_name const&) -> _name& = delete;     \
                                                        \
  _name (_name&&) noexcept;                             \
  auto operator= (_name&&) noexcept -> _name&;          \
                                                        \
  ~_name() noexcept;                                    \
                                                        \
  struct Implementation;                                \
  Implementation *_implementation {nullptr};            \
                                                        \
  MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD              \
  _name (Implementation*) noexcept


// NOLINTNEXTLINE (modernize-concat-nested-namespaces), it is C++11
namespace mcs
{
  namespace share_service
  {
    struct shmem_client
    {
      struct NumberOfThreads
      {
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONSTRUCTOR
          (NumberOfThreads, std::uint32_t);
        MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (NumberOfThreads);
      };

      struct Size
      {
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONSTRUCTOR (Size, std::uint64_t);
        MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (Size);
      };

      struct Chunk
      {
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (ok) -> bool;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (error) -> char const*;

        // de/serialization
        using Text = char const*;
        using TextSize = std::uint64_t;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (text) -> Text;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (text_size) -> TextSize;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONSTRUCTOR (Chunk, Text, TextSize);

        MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (Chunk);
      };

      struct MutableSpan
      {
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (ok) -> bool;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (error) -> char const*;

        // mutable access
        using Data = void*;
        using DataSize = std::uint64_t;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (data) -> Data;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (size) -> DataSize;

        MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (MutableSpan);
      };

      struct ConstSpan
      {
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (ok) -> bool;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (error) -> char const*;

        // const access
        using Data = void const*;
        using DataSize = std::uint64_t;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (data) -> Data;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (size) -> DataSize;

        MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (ConstSpan);
      };

      struct Void
      {
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (ok) -> bool;
        MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (error) -> char const*;

        MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (Void);
      };

      MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (ok) -> bool;
      MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER (error) -> char const*;

      // Returns: A new chunk.
      using Prefix = char const*;
      using AccessMode = std::uint32_t;
      using MLOCKed = bool;
      MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_MEMBER
        ( create
        , Size
        , Prefix
        , AccessMode = 00400 | 00200 // S_IRUSR | S_IWUSR
        , MLOCKed = false
        ) -> Chunk;

      // Returns: A mutable span to access the chunk data.
      // Expects: chunk.ok()
      MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_MEMBER
        (attach_mutable, Chunk const&) -> MutableSpan;

      // Returns: A const span to access the chunk data.
      // Expects: chunk.ok()
      MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_MEMBER
        (attach_const, Chunk const&) -> ConstSpan;

      // Removes the chunk data.
      // Expects: chunk.ok()
      MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_MEMBER
        (remove, Chunk const&) -> Void;

      MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL (shmem_client);
    };

#define MCS_SHARE_SERVICE_MAKE_SHMEM_LOCAL_STREAM_CLIENT(_signals...)   \
    MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD                            \
      auto make_shmem_local_stream_client                               \
        ( shmem_client::NumberOfThreads                                 \
        , char const* socket                                            \
          _signals                                                      \
        ) noexcept -> shmem_client

    MCS_SHARE_SERVICE_MAKE_SHMEM_LOCAL_STREAM_CLIENT
      (
      );
    MCS_SHARE_SERVICE_MAKE_SHMEM_LOCAL_STREAM_CLIENT
      (
      , std::int32_t signal
      );
    MCS_SHARE_SERVICE_MAKE_SHMEM_LOCAL_STREAM_CLIENT
      (
      , std::int32_t signal1
      , std::int32_t signal2
      );
    MCS_SHARE_SERVICE_MAKE_SHMEM_LOCAL_STREAM_CLIENT
      (
      , std::int32_t signal1
      , std::int32_t signal2
      , std::int32_t signal3
      );

#undef MCS_SHARE_SERVICE_MAKE_SHMEM_LOCAL_STREAM_CLIENT

    struct TCPVersion
    {
      struct V4{};
      struct V6{};
    };

#define MCS_SHARE_SERVICE_MAKE_SHMEM_TCP_CLIENT(_signals...)    \
    MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD                    \
      auto make_shmem_tcp_client                                \
        ( shmem_client::NumberOfThreads                         \
        , TCPVersion::V4                                        \
        , std::uint16_t port                                    \
          _signals                                              \
        ) noexcept -> shmem_client;                             \
                                                                \
    MCS_SHARE_SERVICE_SHMEM_CLIENT_NODISCARD                    \
      auto make_shmem_tcp_client                                \
        ( shmem_client::NumberOfThreads                         \
        , TCPVersion::V6                                        \
        , std::uint16_t port                                    \
          _signals                                              \
        ) noexcept -> shmem_client

    MCS_SHARE_SERVICE_MAKE_SHMEM_TCP_CLIENT
      (
      );
    MCS_SHARE_SERVICE_MAKE_SHMEM_TCP_CLIENT
      (
      , std::int32_t signal
      );
    MCS_SHARE_SERVICE_MAKE_SHMEM_TCP_CLIENT
      (
      , std::int32_t signal1
      , std::int32_t signal2
      );
    MCS_SHARE_SERVICE_MAKE_SHMEM_TCP_CLIENT
      (
      , std::int32_t signal1
      , std::int32_t signal2
      , std::int32_t signal3
      );

#undef MCS_SHARE_SERVICE_MAKE_SHMEM_TCP_CLIENT
  }
}

#undef MCS_SHARE_SERVICE_SHMEM_CLIENT_PIMPL
#undef MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_MEMBER
#undef MCS_SHARE_SERVICE_SHMEM_CLIENT_CONST_OBSERVER
#undef MCS_SHARE_SERVICE_SHMEM_CLIENT_CONSTRUCTOR
#endif
