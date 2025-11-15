// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <memory>

namespace mcs::iov_backend
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    template<typename Executor>
      Client<Protocol, AccessPolicy>::Client
        ( Executor& executor
        , util::ASIO::Connectable<Protocol> connectable
        )
          : Base
            { executor
            , connectable
            , std::make_shared<AccessPolicy>()
            }
  {}
}

namespace mcs::iov_backend
{
  template<rpc::is_access_policy AccessPolicy, typename Executor>
    auto make_client
      ( Executor& executor
      , util::ASIO::AnyConnectable any_connectable
      ) -> AnyClient<AccessPolicy>
  {
    return util::ASIO::run
      ( any_connectable
      , [&]<util::ASIO::is_protocol Protocol>
          ( util::ASIO::Connectable<Protocol> connectable
          ) -> AnyClient<AccessPolicy>
        {
          return Client<Protocol, AccessPolicy> {executor, connectable};
        }
      );
  }
}

namespace mcs::iov_backend
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::provider_state
      (
      ) const -> provider::State
  {
    return Base::operator()
      ( command::State{}
      );
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::range
      ( collection::ID collection_id
      ) const -> core::memory::Range
  {
    return Base::operator()
      ( command::Range {collection_id}
      );
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::locations
      ( collection::ID collection_id
      , core::memory::Range range
      ) const -> Locations
  {
    return Base::operator()
      ( command::Locations {collection_id, range}
      );
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::collection_create
      ( collection::ID collection_id
      , core::memory::Size size
      ) const -> UsedStorages
  {
    return Base::operator()
      ( command::collection::Create {collection_id, size}
      );
  }
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::collection_append
      ( collection::ID collection_id
      , core::memory::Range range_to_append
      ) const -> core::memory::Size
  {
    return Base::operator()
      ( command::collection::Append {collection_id, range_to_append}
      );
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::collection_delete
      ( collection::ID collection_id
      ) const -> void
  {
    return Base::operator()
      ( command::collection::Delete {collection_id}
      );
  }

  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    auto Client<Protocol, AccessPolicy>::add
      ( Storage storage
      ) const -> storage::ID
  {
    return Base::operator()
      ( command::storage::Add {storage}
      );
  }
}
