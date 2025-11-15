// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <gtest/gtest.h>
#include <iterator>
#include <mcs/core/Storages.hpp>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/testing/RPC/ProtocolState.hpp>
#include <mcs/testing/UniqTemporaryDirectory.hpp>
#include <mcs/testing/core/operator==/storage/MaxSize.hpp>
#include <mcs/testing/core/printer/storage/MaxSize.hpp>
#include <mcs/testing/core/random/storage/MaxSize.hpp>
#include <mcs/testing/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/storage/implementation/SHMEM.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/require_exception.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/read/read.hpp>
#include <string>

namespace mcs::core
{
  namespace
  {
    using Protocols = ::testing::Types
      < asio::ip::tcp
      , asio::local::stream_protocol
      >;
    template<class> struct MCSCoreManageStorageInRemoteProcessT
      : public testing::random::Test{}
      ;
    TYPED_TEST_SUITE (MCSCoreManageStorageInRemoteProcessT, Protocols);
  }

  // Publish the ID of the storage provider and let remote processes
  // add and remove storage. Verify the storage has been added/removed
  // by accessing its max_size.
  //
  TYPED_TEST ( MCSCoreManageStorageInRemoteProcessT
             , remote_process_can_manage_storage_of_storages
             )
  {
    using Protocol = TypeParam;

    auto storages {core::Storages<SupportedStorageImplementations>{}};

    // create a storage provider for remote access
    auto io_context_provider
      { rpc::ScopedRunningIOContext
          {rpc::ScopedRunningIOContext::NumberOfThreads {1u}, SIGINT, SIGTERM}
      };
    auto const protocol_state {testing::RPC::ProtocolState<Protocol>{}};
    auto const provider
      { control::Provider<Protocol, SupportedStorageImplementations>
          { io_context_provider
          , protocol_state.local_endpoint()
          , std::addressof (storages)
          }
      };

    auto const temporary_directory
      { testing::UniqTemporaryDirectory
          {"MCS_TEST_CORE_TEST_MANAGE_STORAGE_IN_REMOTE_PROCESS_TEST"}
      };
    auto const storage_id_file {temporary_directory.path() / "storage_id"};

    auto const max_size {testing::random::value<storage::MaxSize>{}()};

    ASSERT_EQ
      ( EXIT_SUCCESS
      , std::system
        ( fmt::format
          ( "{} '{}' '{}' '{}'"
          , MCS_TEST_CORE_TEST_MANAGE_STORAGE_IN_REMOTE_PROCESS_STORAGE_CREATE
          , util::ASIO::make_connectable (provider.local_endpoint())
          , max_size
          , storage_id_file
          ).c_str()
        )
      );

    auto const storage_id
      {util::read::from_file<storage::ID> (storage_id_file)};

    ASSERT_EQ ( max_size
              , storages
                . template size_max<typename storage::implementation::SHMEM>
                    ( storages.read_access()
                    , storage_id
                    , storage::implementation::SHMEM::Parameter::Size::Max{}
                    )
              );

    ASSERT_EQ
      ( EXIT_SUCCESS
      , std::system
        ( fmt::format
          ( "{} '{}' '{}'"
          , MCS_TEST_CORE_TEST_MANAGE_STORAGE_IN_REMOTE_PROCESS_STORAGE_REMOVE
          , util::ASIO::make_connectable (provider.local_endpoint())
          , storage_id
          ).c_str()
        )
      );

    testing::require_exception
      ( [&]
        {
          std::ignore = storages
            . template size_max<storage::implementation::SHMEM>
              ( storages.read_access()
              , storage_id
              , storage::implementation::SHMEM::Parameter::Size::Max{}
              );
        }
      , testing::assert_type_and_what<typename decltype (storages)::Error::UnknownID>
          (fmt::format ("Unknown id '{}'", storage_id))
      );
  }
}
