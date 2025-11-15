// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/iov_backend/Commands.hpp>
#include <mcs/iov_backend/Locations.hpp>
#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <variant>

namespace mcs::iov_backend
{
  template<rpc::is_protocol Protocol, rpc::is_access_policy AccessPolicy>
    struct Client : public Commands::template wrap< rpc::Client
                                                  , Protocol
                                                  , AccessPolicy
                                                  >
  {
    using Base = Commands::template wrap<rpc::Client, Protocol, AccessPolicy>;

    template<typename Executor>
      Client ( Executor&
             , util::ASIO::Connectable<Protocol>
             );

    auto provider_state() const -> provider::State;

    auto range (collection::ID) const -> core::memory::Range;
    auto locations (collection::ID, core::memory::Range) const -> Locations;

    auto add (Storage) const -> storage::ID;

    auto collection_create
      ( collection::ID
      , core::memory::Size
      ) const -> UsedStorages
      ;
    auto collection_append
      ( collection::ID
      , core::memory::Range
      ) const -> core::memory::Size
      ;

    auto collection_delete
      ( collection::ID
      ) const -> void
      ;
  };

  template<rpc::is_access_policy AccessPolicy>
    using AnyClient = std::variant
      < Client<asio::ip::tcp, AccessPolicy>
      , Client<asio::local::stream_protocol, AccessPolicy>
      >
    ;

  template<rpc::is_access_policy AccessPolicy, typename Executor>
    auto make_client
      ( Executor&
      , util::ASIO::AnyConnectable
      ) -> AnyClient<AccessPolicy>
    ;
}

#include "detail/Client.ipp"
