// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <future>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/util/type/List.hpp>
#include <type_traits>

namespace mcs::block_device
{
  template<typename MakeTransportClient, typename MetaDataClient>
    struct Writer
  {
    Writer (MetaDataClient&, MakeTransportClient&&);

    struct Create{};
    template<typename... Args>
      requires (std::is_constructible_v<MakeTransportClient, Args...>)
      Writer (Create, MetaDataClient&, Args&&...);

    // Write the data behind source address into the block with the
    // block::ID. The memory behind the source address must provide
    // enough data to read a full block and must be available until
    // the write operation has finished. The writer will overwrite the
    // content of the block with the block::ID.
    //
    template<core::storage::is_implementation... StorageImplementations>
      auto operator()
        ( block::ID
        , core::transport::Address const& source
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
    [[nodiscard]] auto make_writer
      ( MetaDataClient&
      , Args&&...
      );
}

#include "detail/Writer.ipp"
