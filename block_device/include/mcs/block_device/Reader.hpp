// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <future>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/Address.hpp>
#include <type_traits>

namespace mcs::block_device
{
  template<typename MakeTransportClient, typename MetaDataClient>
    struct Reader
  {
    Reader (MetaDataClient&, MakeTransportClient&&);

    struct Create{};
    template<typename... Args>
      requires (std::is_constructible_v<MakeTransportClient, Args...>)
      Reader (Create, MetaDataClient&, Args&&...);

    // Read the content of the block with the block::ID into the
    // destination address. The memory behind the destination address
    // must provide enough space to store a full block and must be
    // available until the read has finished. The reader will
    // overwrite the memory behind the destination address.
    //
    template<core::storage::is_implementation... StorageImplementations>
      auto operator()
        ( block::ID
        , core::transport::Address const& destination
        ) -> std::future<core::memory::Size>
        ;

  private:
    MetaDataClient& _meta_data_client;
    MakeTransportClient _make_transport_client;
  };

  template< typename MakeTransportClient
          , typename MetaDataClient
          , typename... Args
          >
    requires (std::is_constructible_v<MakeTransportClient, Args...>)
    [[nodiscard]] auto make_reader
      ( MetaDataClient&
      , Args&&...
      );
}

#include "detail/Reader.ipp"
