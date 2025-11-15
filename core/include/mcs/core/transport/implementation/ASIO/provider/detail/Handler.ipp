// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <fmt/format.h>
#include <functional>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/connected_socket.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace mcs::core::transport::implementation::ASIO::provider
{
  template<storage::is_implementation... StorageImplementations>
    Handler<StorageImplementations...>::Handler
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      )
        : _storages {storages}
  {}

  template<storage::is_implementation... StorageImplementations>
    template<typename Socket>
      auto Handler<StorageImplementations...>::operator()
        ( command::Get get
        , Socket& socket
        ) const -> command::Get::Response
  {
    auto const chunk
      { make_chunk<chunk::access::Const>
        ( _storages
        , get.source.storage_id
        , get.source.storage_parameter_chunk_description
        , get.source.segment_id
        , memory::make_range (get.source.offset, get.size)
        )
      };
    auto const data {chunk.data()};
    auto const bytes_written {asio::write (socket, asio::buffer (data))};

    if (bytes_written != data.size())
    {
      throw typename Error::CouldNotWriteAllData
        { typename Error::CouldNotWriteAllData::Wanted {data.size()}
        , typename Error::CouldNotWriteAllData::Written {bytes_written}
        };
    }

    return memory::make_size (data.size());
  }

  template<storage::is_implementation... StorageImplementations>
    template<typename Socket>
      auto Handler<StorageImplementations...>::operator()
        ( command::Put put
        , Socket& socket
        ) const -> command::Put::Response
  {
    auto const size {std::get<std::size_t> (put.bytes_or_size)};
    auto const chunk
      { make_chunk<chunk::access::Mutable>
        ( _storages
        , put.destination.storage_id
        , put.destination.storage_parameter_chunk_description
        , put.destination.segment_id
        , memory::make_range (put.destination.offset, memory::make_size (size))
        )
      };
    auto const sink {as<std::byte> (chunk)};

    auto const bytes_read
      {asio::read (socket, asio::buffer (sink.data(), size))};

    if (bytes_read != size)
    {
      throw typename Error::CouldNotReadAllData
        { typename Error::CouldNotReadAllData::Wanted {size}
        , typename Error::CouldNotReadAllData::Read {bytes_read}
        };
    }

    return memory::make_size (size);
  }

  template<storage::is_implementation... StorageImplementations>
    Handler<StorageImplementations...>::Error::CouldNotWriteAllData::CouldNotWriteAllData
      ( Wanted wanted
      , Written written
      ) noexcept
        : mcs::Error
          { fmt::format
            ( "mcs::core::transport::implementation::ASIO::provider::Handler::CouldNotWriteAllData:"
              " wanted: {}, written: {}"
            , wanted.value
            , written.value
            )
          }
        , _wanted {wanted}
        , _written {written}
  {}
  template<storage::is_implementation... StorageImplementations>
    Handler<StorageImplementations...>::Error::CouldNotWriteAllData::~CouldNotWriteAllData
      (
      ) = default
    ;

  template<storage::is_implementation... StorageImplementations>
    constexpr Handler<StorageImplementations...>::Error::CouldNotWriteAllData::Wanted::Wanted
      ( std::size_t value_
      ) noexcept
        : value {value_}
  {}
  template<storage::is_implementation... StorageImplementations>
    constexpr Handler<StorageImplementations...>::Error::CouldNotWriteAllData::Written::Written
      ( std::size_t value_
      ) noexcept
        : value {value_}
  {}
  template<storage::is_implementation... StorageImplementations>
    constexpr auto Handler<StorageImplementations...>::Error::CouldNotWriteAllData::wanted
      (
      ) const noexcept -> Wanted
  {
    return _wanted;
  }
  template<storage::is_implementation... StorageImplementations>
    constexpr auto Handler<StorageImplementations...>::Error::CouldNotWriteAllData::written
      (
      ) const noexcept -> Written
  {
    return _written;
  }

  template<storage::is_implementation... StorageImplementations>
    Handler<StorageImplementations...>::Error::CouldNotReadAllData::CouldNotReadAllData
      ( Wanted wanted
      , Read read
      ) noexcept
        : mcs::Error
          { fmt::format
            ( "mcs::core::transport::implementation::ASIO::provider::Handler::CouldNotReadAllData:"
              " wanted: {}, written: {}"
            , wanted.value
            , read.value
            )
          }
        , _wanted {wanted}
        , _read {read}
  {}
  template<storage::is_implementation... StorageImplementations>
    Handler<StorageImplementations...>::Error::CouldNotReadAllData::~CouldNotReadAllData
      (
      ) = default
    ;
  template<storage::is_implementation... StorageImplementations>
    constexpr Handler<StorageImplementations...>::Error::CouldNotReadAllData::Wanted::Wanted
      ( std::size_t value_
      ) noexcept
        : value {value_}
  {}
  template<storage::is_implementation... StorageImplementations>
    constexpr Handler<StorageImplementations...>::Error::CouldNotReadAllData::Read::Read
      ( std::size_t value_
      ) noexcept
        : value {value_}
  {}
  template<storage::is_implementation... StorageImplementations>
    constexpr auto Handler<StorageImplementations...>::Error::CouldNotReadAllData::wanted
      (
      ) const noexcept -> Wanted
  {
    return _wanted;
  }
  template<storage::is_implementation... StorageImplementations>
    constexpr auto Handler<StorageImplementations...>::Error::CouldNotReadAllData::read
      (
      ) const noexcept -> Read
  {
    return _read;
  }
}
