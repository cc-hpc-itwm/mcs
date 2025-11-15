// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/Client.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <memory>

namespace mcs::core::transport::implementation::ASIO
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    template<typename Executor>
      Client< Protocol
            , AccessPolicy
            , util::type::List<StorageImplementations...>
            >::Client
        ( Executor& io_context
        , util::ASIO::Connectable<Protocol> provider_connectable
        , util::not_null<Storages<util::type::List<StorageImplementations...>>>
            storages
        )
          : Base
            { io_context
            , provider_connectable
            , std::make_shared<AccessPolicy>()
            }
          , _storages {storages}
  {}
}

namespace mcs::core::transport::implementation::ASIO
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::memory_get
      ( Address destination
      , Address source
      , memory::Size size
      ) const -> std::future<memory::Size>
  {
    return Base::get_future
      ( command::Get
          ( source
          , size
          , std::unique_ptr<command::Get::Destination>
              { new Destination
                { _storages
                , destination
                , size
                }
              }
          )
      );
  }

  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::memory_put
      ( Address destination
      , Address source
      , memory::Size size
      ) const -> std::future<memory::Size>
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

    return Base::get_future
      ( command::Put
        { destination
        , as<std::byte const> (chunk)
        }
      );
  }
}

namespace mcs::core::transport::implementation::ASIO
{
  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
      Client< Protocol
            , AccessPolicy
            , util::type::List<StorageImplementations...>
            >::Destination::Destination
        ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
            storages
        , Address destination
        , memory::Size size
        )
          : _chunk
            { make_chunk<chunk::access::Mutable>
              ( storages
              , destination.storage_id
              , destination.storage_parameter_chunk_description
              , destination.segment_id
              , memory::make_range (destination.offset, size)
              )
            }
  {}

  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    auto Client< Protocol
               , AccessPolicy
               , util::type::List<StorageImplementations...>
               >::Destination::data() const -> std::span<std::byte>
  {
    return _chunk.data();
  }
}
