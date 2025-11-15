// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/rpc/Client.hpp>
#include <memory>
#include <utility>

namespace mcs::share_service
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    template<typename Executor>
    Client<Protocol, AccessPolicy>::Client
      ( Executor& executor
      , util::ASIO::Connectable<Protocol> provider_connectable
      )
        : Base
          { executor
          , provider_connectable
          , std::make_shared<AccessPolicy>()
          }
        , _provider_connectable {provider_connectable}
  {}

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    template<is_supported_storage_implementation StorageImplementation>
      auto Client<Protocol, AccessPolicy>::create
        ( core::memory::Size size
        , command::create::Parameters<StorageImplementation>
            parameters
        ) const -> Chunk
  {
    auto const [storage_id, storage_implementation_id, segment_id]
      { Base::operator()
          ( command::Create
            { size
            , parameters
            }
          )
      };

    return Chunk
      { util::ASIO::AnyConnectable {_provider_connectable}
      , storage_id
      , storage_implementation_id
      , segment_id
      , size
      };
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    template< core::chunk::is_access Access
            , is_supported_storage_implementation StorageImplementation
            >
      auto Client<Protocol, AccessPolicy>::attach
        ( Chunk chunk
        , command::attach::Parameters<StorageImplementation>
            parameters
        ) const -> SupportedStorageImplementations::template wrap< core::Chunk
                                                                 , Access
                                                                 >
  {
    return SupportedStorageImplementations::template wrap<core::Chunk, Access>
      { Base::operator()
        ( command::Attach<Access>
          { chunk
          , parameters
          }
        )
      };
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    template<is_supported_storage_implementation StorageImplementation>
      auto Client<Protocol, AccessPolicy>::remove
        ( Chunk chunk
        , command::remove::Parameters<StorageImplementation>
            parameters
        ) const -> void
  {
    return Base::operator()
      ( command::Remove
        { chunk
        , parameters
        }
      );
  }
}
