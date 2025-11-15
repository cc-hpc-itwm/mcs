// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/signal_set.hpp>
#include <concepts>
#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <functional>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/block_device/meta_data/Blocks.hpp>
#include <mcs/block_device/meta_data/Client.hpp>
#include <mcs/block_device/meta_data/Provider.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace
{
  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::Heap
    , mcs::core::storage::implementation::SHMEM
    >;

  template<typename SI>
    concept is_supported_storage_implementation
      = SupportedStorageImplementations::template contains<SI>()
    ;

  template<is_supported_storage_implementation Implementation>
    struct Create
  {
    using type = Implementation::Parameter::Create;
  };

  auto storage_provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 7)
    {
      throw std::invalid_argument
        { fmt::format
            ( "usage: {} meta_data_provider_path storage_endpoint storage_provider_path number_of_threads size STORAGE"
            , args[0]
            )
         };
    }

    auto storages {mcs::core::Storages<SupportedStorageImplementations>{}};

    auto const size {mcs::util::read::read<mcs::core::memory::Size> (args[5])};

    return SupportedStorageImplementations::template parse_id_and_run<Create>
      ( args[6]
      , [&]<is_supported_storage_implementation StorageImplementation>
          (typename StorageImplementation::Parameter::Create create)
        {
          auto const storage
            { mcs::core::make_unique_storage<StorageImplementation>
                ( std::addressof (storages)
                , create
                )
            };
          auto const segment
            { mcs::core::storage::make_unique_segment<StorageImplementation>
              ( std::addressof (storages)
              , storage->id()
              , size
              )
            };

    auto const meta_data_provider_path {std::filesystem::path (args[1])};
    auto const meta_data_provider_connectable
      { mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (meta_data_provider_path / "PROVIDER")
      };

    auto io_context_transport_provider
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            {mcs::util::read::read<unsigned int> (args[4])}
        }
      };
    auto io_context_meta_data
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[2])
      , [&]<mcs::util::ASIO::is_protocol StorageProtocol>
          (typename StorageProtocol::endpoint storage_endpoint)
        {
          using TransportProviderImplementation
            = mcs::core::transport::implementation::ASIO::Provider
                < StorageProtocol
                , SupportedStorageImplementations
                >
            ;

          auto transport_provider
            { TransportProviderImplementation
              { io_context_transport_provider
              , storage_endpoint
              , std::addressof (storages)
              }
            };

          auto const block_storage
            { mcs::block_device::Storage
              { transport_provider.connection_information()
              , storage->id()
              , mcs::core::storage::make_parameter
                  ( typename StorageImplementation
                      ::Parameter::Chunk::Description{}
                  )
              , segment->id()
              , mcs::core::memory::make_range
                  (mcs::core::memory::make_offset (0), size)
              }
            };

          return mcs::util::ASIO::run
            ( meta_data_provider_connectable
            , [&]<mcs::util::ASIO::is_protocol MetaDataProtocol>
                (mcs::util::ASIO::Connectable<MetaDataProtocol> meta_data_provider)
              {
                auto meta_data_client
                  { mcs::block_device::meta_data::Client
                      < MetaDataProtocol
                      , mcs::rpc::access_policy::Exclusive
                      >
                    { io_context_meta_data
                    , meta_data_provider
                    }
                  };

                auto const add_result {meta_data_client.add (block_storage)};

                auto signals
                  { asio::signal_set { io_context_transport_provider
                                     , SIGINT, SIGTERM
                                     }
                  };
                signals.async_wait
                  ( [&] (auto, auto)
                    {
                      if (add_result.blocks.has_value())
                      {
                        auto const remove_result
                          {meta_data_client.remove (*add_result.blocks)};

                        fmt::print ("reclaimed {}\n", remove_result.unused);
                      }

                      io_context_transport_provider.stop();
                    }
                  );

                if (add_result.blocks.has_value())
                {
                  fmt::print ("new blocks: {}\n", *add_result.blocks);
                }
                if (add_result.unused.has_value())
                {
                  fmt::print ("unused memory: {}\n", *add_result.unused);
                }

                auto const temporary_directory
                  {mcs::util::TemporaryDirectory {args[3]}};

                auto const pid_information
                  { mcs::util::TemporaryFile
                    { temporary_directory.path() / "PID"
                    , "{}"
                    , mcs::util::syscall::getpid()
                    }
                  };

                // run "for ever"
                io_context_transport_provider.join();

                // \todo daemonize and proper return once the connection
                // information has been written

                return EXIT_SUCCESS;
              }
            );
        }
      );
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, storage_provider_main);
}
