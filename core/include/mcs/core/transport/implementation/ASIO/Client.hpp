// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <future>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/ASIO/Commands.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::transport::implementation::ASIO
{
  // \todo why is Concurrent not supported?
  template<typename AccessPolicy>
    concept is_supported_access_policy = rpc::is_access_policy<AccessPolicy>
    && (  std::is_same_v<AccessPolicy, rpc::access_policy::Exclusive>
       || std::is_same_v<AccessPolicy, rpc::access_policy::Sequential>
       )
    ;

  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , typename StorageImplementations
          > struct Client;

  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , storage::is_implementation... StorageImplementations
          >
    struct Client< Protocol
                 , AccessPolicy
                 , util::type::List<StorageImplementations...>
                 >
    : public Commands::template wrap< rpc::Client
                                    , Protocol
                                    , AccessPolicy
                                    >
  {
    using Base = Commands::template wrap< rpc::Client
                                        , Protocol
                                        , AccessPolicy
                                        >
      ;

    template<typename Executor>
      explicit Client
        ( Executor&
        , util::ASIO::Connectable<Protocol>
        , util::not_null<Storages<util::type::List<StorageImplementations...>>>
        );

    auto memory_get
      ( Address destination
      , Address source
      , memory::Size
      ) const -> std::future<memory::Size>
      ;

    auto memory_put
      ( Address destination
      , Address source
      , memory::Size
      ) const -> std::future<memory::Size>
      ;

  private:
    util::not_null<Storages<util::type::List<StorageImplementations...>>>
      _storages;

    struct Destination final : public command::Get::Destination
    {
      explicit Destination
        ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
        , Address
        , memory::Size
        );

      auto data() const -> std::span<std::byte> override;

    private:
      Chunk<chunk::access::Mutable, StorageImplementations...> _chunk;
    };
  };
}

#include "detail/Client.ipp"
