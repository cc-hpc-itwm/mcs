// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <fmt/format.h>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/chunk/access/Const.hpp>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/connected_socket.hpp>

namespace mcs::core::transport::implementation::libfabric::provider
{
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    Handler<Transporter, StorageImplementations...>::Handler
      ( util::not_null<Transporter> transporter
      , util::not_null<Storages<util::type::List<StorageImplementations...>>>
          storages
      )
        : _transporter {transporter}
        , _storages {storages}
  {}

  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    auto Handler<Transporter, StorageImplementations...>::operator()
      ( command::Get get
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
    _transporter->send (data, get.name);

    return memory::make_size (data.size());
  }

  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    auto Handler<Transporter, StorageImplementations...>::operator()
      ( command::Put put
      ) const -> command::Put::Response
  {
    auto const chunk
      { make_chunk<chunk::access::Mutable>
        ( _storages
        , put.destination.storage_id
        , put.destination.storage_parameter_chunk_description
        , put.destination.segment_id
        , memory::make_range (put.destination.offset, put.size)
        )
      };
    auto const data {chunk.data()};
    auto const bytes_read {_transporter->recv (data)};

    if (bytes_read != data.size())
    {
      throw typename Error::CouldNotReadAllData
        { typename Error::CouldNotReadAllData::Wanted {data.size()}
        , typename Error::CouldNotReadAllData::Read {bytes_read}
        };
    }

    return memory::make_size (data.size());
  }

  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    Handler
      < Transporter
      , StorageImplementations...
      >::Error::CouldNotReadAllData::CouldNotReadAllData
        ( Wanted wanted
        , Read read
        ) noexcept
          : mcs::Error
            { fmt::format
              ( "mcs::core::transport::implementation::libfabric::provider::"
                "Handler::CouldNotReadAllData:"
                " wanted: {}, read: {}"
              , wanted.value
              , read.value
              )
            }
          , _wanted {wanted}
          , _read {read}
  {}
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    Handler
      < Transporter
      , StorageImplementations...
      >::Error::CouldNotReadAllData::~CouldNotReadAllData
        (
        ) = default
    ;
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr Handler
      < Transporter
      , StorageImplementations...
      >::Error::CouldNotReadAllData::Wanted::Wanted
        ( std::size_t value_
        ) noexcept
          : value {value_}
  {}
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr Handler
      < Transporter
      , StorageImplementations...
      >::Error::CouldNotReadAllData::Read::Read
        ( std::size_t value_
        ) noexcept
          : value {value_}
  {}
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr auto Handler
      < Transporter
      , StorageImplementations...
      >::Error::CouldNotReadAllData::wanted
        (
        ) const noexcept -> Wanted
  {
    return _wanted;
  }
  template< transporter::is_provider Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr auto Handler
      < Transporter
      , StorageImplementations...
      >::Error::CouldNotReadAllData::read
        (
        ) const noexcept -> Read
  {
    return _read;
  }
}
