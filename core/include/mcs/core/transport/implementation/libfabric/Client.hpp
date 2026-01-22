// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <future>
#include <mcs/Error.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/libfabric/Commands.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/provider/ConnectionInformation.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/Concepts.hpp>
#include <mcs/rpc/Client.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/rpc/access_policy/Sequential.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::transport::implementation::libfabric
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
          , transporter::is_client Transporter
          , typename StorageImplementations
          > struct Client;

  template< util::ASIO::is_protocol Protocol
          , is_supported_access_policy AccessPolicy
          , transporter::is_client Transporter
          , storage::is_implementation... StorageImplementations
          >
    struct Client< Protocol
                 , AccessPolicy
                 , Transporter
                 , util::type::List<StorageImplementations...>
                 >
  {
    struct Error
    {
      struct CouldNotReadAllData : public mcs::Error
      {
        struct Wanted
        {
          constexpr explicit Wanted (std::size_t) noexcept;
          std::size_t value;
        };
        struct Read
        {
          constexpr explicit Read (std::size_t) noexcept;
          std::size_t value;
        };

        [[nodiscard]] constexpr auto wanted() const noexcept -> Wanted;
        [[nodiscard]] constexpr auto read() const noexcept -> Read;

        ~CouldNotReadAllData() override;
        CouldNotReadAllData (CouldNotReadAllData const&) = default;
        CouldNotReadAllData (CouldNotReadAllData&&) noexcept = default;
        auto operator= (CouldNotReadAllData const&) -> CouldNotReadAllData& = default;
        auto operator= (CouldNotReadAllData&&) noexcept  -> CouldNotReadAllData& = default;

      private:
       friend struct Client;

        CouldNotReadAllData (Wanted, Read) noexcept;

        Wanted _wanted;
        Read _read;
      };
    };

    template<typename Executor>
      explicit Client
        ( Executor&
        , provider::ConnectionInformation<Protocol> connection_information
        , libfabric::Interface
        , util::not_null<Storages<util::type::List<StorageImplementations...>>>
        );

    auto memory_get
      ( Address destination
      , Address source
      , memory::Size
      ) -> std::future<memory::Size>
      ;

    auto memory_put
      ( Address destination
      , Address source
      , memory::Size
      ) -> std::future<memory::Size>
      ;

  private:
    Commands::template wrap
      < rpc::Client
      , Protocol
      , AccessPolicy
      > _controller
      ;

    Transporter _transporter;

    util::not_null<Storages<util::type::List<StorageImplementations...>>>
      _storages;
  };
}

#include "detail/Client.ipp"
