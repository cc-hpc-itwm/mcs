// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/Chunk.hpp>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <mcs/core/transport/implementation/libfabric/command/Get.hpp>
#include <mcs/core/transport/implementation/libfabric/command/Put.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/util/ASIO/Connectable.hpp>

namespace mcs::core::transport::implementation::libfabric
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr Client
      < Protocol
      , AccessPolicy
      , Transporter
      , util::type::List<StorageImplementations...>
      >::Error::CouldNotReadAllData::Wanted::Wanted
        ( std::size_t value_
        ) noexcept
          : value {value_}
  {}
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr Client
      < Protocol
      , AccessPolicy
      , Transporter
      , util::type::List<StorageImplementations...>
      >::Error::CouldNotReadAllData::Read::Read
        ( std::size_t value_
        ) noexcept
          : value {value_}
  {}
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr auto Client
      < Protocol
      , AccessPolicy
      , Transporter
      , util::type::List<StorageImplementations...>
      >::Error::CouldNotReadAllData::wanted
        (
        ) const noexcept -> Wanted
  {
    return _wanted;
  }
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    constexpr auto Client
      < Protocol
      , AccessPolicy
      , Transporter
      , util::type::List<StorageImplementations...>
      >::Error::CouldNotReadAllData::read
        (
        ) const noexcept -> Read
  {
    return _read;
  }
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    Client
      < Protocol
      , AccessPolicy
      , Transporter
      , util::type::List<StorageImplementations...>
      >::Error::CouldNotReadAllData::CouldNotReadAllData
        ( Wanted wanted
        , Read read
        ) noexcept
          : mcs::Error
            { fmt::format
              ( "mcs::core::transport::implementation::libfabric::Client::"
                "Error::CouldNotReadAllData:"
                " wanted: {}, read: {}"
              , wanted.value
              , read.value
              )
            }
          , _wanted {wanted}
          , _read {read}
  {}
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    Client
      < Protocol
      , AccessPolicy
      , Transporter
      , util::type::List<StorageImplementations...>
      >::Error::CouldNotReadAllData::~CouldNotReadAllData() = default;
}

namespace mcs::core::transport::implementation::libfabric
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    template<typename Executor>
      Client< Protocol
            , AccessPolicy
            , Transporter
            , util::type::List<StorageImplementations...>
            >::Client
        ( Executor& io_context
        , provider::ConnectionInformation<Protocol> connection_information
        , libfabric::Interface transporter_interface
        , util::not_null<Storages<util::type::List<StorageImplementations...>>>
            storages
        )
          : _controller
            { io_context
            , connection_information.control
            , std::make_shared<AccessPolicy>()
            }
          , _transporter
            { transporter_interface
            , connection_information.transport.value()
            }
          , _storages {storages}
  {}
}

namespace mcs::core::transport::implementation::libfabric
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , Transporter
               , util::type::List<StorageImplementations...>
               >::memory_get
      ( Address destination
      , Address source
      , memory::Size size
      ) -> std::future<memory::Size>
  {
    auto const chunk
      { make_chunk<chunk::access::Mutable>
        ( _storages
        , destination.storage_id
        , destination.storage_parameter_chunk_description
        , destination.segment_id
        , memory::make_range (destination.offset, size)
        )
      };

    auto command
      { _controller.get_future
          ( command::Get
              { source
              , size
              , _transporter.name()
              }
          )
      };

    auto const data {chunk.data()};
    auto const bytes_read {_transporter.recv (data)};

    if (bytes_read != data.size())
    {
      throw typename Error::CouldNotReadAllData
        { typename Error::CouldNotReadAllData::Wanted {data.size()}
        , typename Error::CouldNotReadAllData::Read {bytes_read}
        };
    }

    return command;
  }
}

namespace mcs::core::transport::implementation::libfabric
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , Transporter
               , util::type::List<StorageImplementations...>
               >::memory_put
      ( Address destination
      , Address source
      , memory::Size size
      ) -> std::future<memory::Size>
  {
    auto const chunk
      { make_chunk<chunk::access::Const>
        ( _storages
        , source.storage_id
        , source.storage_parameter_chunk_description
        , source.segment_id
        , memory::make_range (source.offset, size)
        )
      };

    auto command {_controller.get_future (command::Put {destination, size})};

    auto const data {chunk.data()};
    _transporter.send (data);

    return command;
  }
}
