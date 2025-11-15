// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/control/Provider.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/command/storage/Add.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/STD/optional.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <mcs/util/syscall/hostname.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>

namespace
{
  template<mcs::iov_backend::is_supported_storage_implementation Implementation>
    struct Create
  {
    using type = Implementation::Parameter::Create;
  };

  auto storage_provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 14)
    {
      throw std::invalid_argument
        { fmt::format
            ( "usage: {}"
            " {{'Just' (provider) | 'Nothing'}}"     // 1
            " storages_provider_endpoint"            // 2
            " storages_provider_number_of_threads"   // 3
            " transport_provider_endpoint "          // 4
            " transport_provider_number_of_threads"  // 5
            " STORAGE"                               // 6
            " parameter_size_max"                    // 7
            " parameter_size_used"                   // 8
            " parameter_segment_create"              // 9
            " parameter_segment_remove"              // 10
            " parameter_chunk_description"           // 11
            " parameter_file_read"                   // 12
            " parameter_file_write"                  // 13
            , args[0]
            )
         };
    }

    auto const add_to_provider_or_restart
      {mcs::util::read::read<std::optional<mcs::util::ASIO::AnyConnectable>> (args[1])};
    auto const storages_provider_endpoint
      {mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[2])};
    auto const storages_provider_number_of_threads
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
         { mcs::util::read::read<unsigned int> (args[3])
         }
      };
    auto const transport_provider_endpoint
      {mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[4])};
    auto const transport_provider_number_of_threads
      { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
         { mcs::util::read::read<unsigned int> (args[5])
         }
      };
    auto const STORAGE {args[6]};

    auto storages
      { mcs::core::Storages<mcs::iov_backend::SupportedStorageImplementations>{}
      };

    return mcs::iov_backend::SupportedStorageImplementations
      ::template parse_id_and_run<Create>
    ( STORAGE
    , [&]< mcs::iov_backend::is_supported_storage_implementation
             StorageImplementation
         >
        ( typename StorageImplementation::Parameter::Create parameter_create
        )
      {
        using Parameter = typename StorageImplementation::Parameter;
        auto const parameter_size_max
          { mcs::util::read::read<typename Parameter::Size::Max> (args[7])
          };
        auto const parameter_size_used
          { mcs::util::read::read<typename Parameter::Size::Used> (args[8])
          };
        auto const parameter_segment_create
          { mcs::util::read::read<typename Parameter::Segment::Create> (args[9])
          };
        auto const parameter_segment_remove
          { mcs::util::read::read<typename Parameter::Segment::Remove> (args[10])
          };
        auto const parameter_chunk_description
          { mcs::util::read::read<typename Parameter::Chunk::Description> (args[11])
          };
        auto const parameter_file_read
          { mcs::util::read::read<typename Parameter::File::Read> (args[12])
          };
        auto const parameter_file_write
          { mcs::util::read::read<typename Parameter::File::Write> (args[13])
          };

        auto const storage
          { mcs::core::make_unique_storage<StorageImplementation>
              ( std::addressof (storages)
              , parameter_create
              )
          };

    auto io_context_storages_provider
      { mcs::rpc::ScopedRunningIOContext
        { storages_provider_number_of_threads
        , SIGINT, SIGTERM
        }
      };

    auto io_context_transport_provider
      { mcs::rpc::ScopedRunningIOContext
        { transport_provider_number_of_threads
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( storages_provider_endpoint
      , [&]<mcs::util::ASIO::is_protocol StoragesProtocol>
          (typename StoragesProtocol::endpoint storages_endpoint)
        {
          auto const storages_provider
            { mcs::core::control::Provider
                 < StoragesProtocol
                 , mcs::iov_backend::SupportedStorageImplementations
                 >
              { io_context_storages_provider
              , storages_endpoint
              , std::addressof (storages)
              }
            };

          return mcs::util::ASIO::run
            ( transport_provider_endpoint
            , [&]<mcs::util::ASIO::is_protocol TransportProtocol>
                (typename TransportProtocol::endpoint transport_endpoint)
              {
                using TransportProvider
                  = mcs::core::transport::implementation::ASIO::Provider
                      < TransportProtocol
                      , mcs::iov_backend::SupportedStorageImplementations
                      >
                  ;

                auto transport_provider
                  { TransportProvider
                    { io_context_transport_provider
                    , transport_endpoint
                    , std::addressof (storages)
                    }
                  };

                auto const transport_provider_connectable
                  { transport_provider.connection_information()
                  };

                if (auto provider {add_to_provider_or_restart})
                {
                  mcs::util::ASIO::run
                    ( *provider
                    , [&]<mcs::util::ASIO::is_protocol IOVBackendProviderProtocol>
                        ( mcs::util::ASIO::Connectable<IOVBackendProviderProtocol>
                            iov_backend_provider
                        )
                      {
                        auto io_context_iov_backend_client
                          { mcs::rpc::ScopedRunningIOContext
                            { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
                            , SIGINT, SIGTERM
                            }
                          };

                        auto iov_backend_client
                          { mcs::iov_backend::Client
                                < IOVBackendProviderProtocol
                                , mcs::rpc::access_policy::Exclusive
                                >
                              ( io_context_iov_backend_client
                              , iov_backend_provider
                              )
                          };

                        auto const storage_id
                          { iov_backend_client.add
                            ( mcs::iov_backend::Storage
                              { storage->implementation_id()
                              , mcs::core::storage::make_parameter (parameter_create)
                              , mcs::util::ASIO::make_connectable
                                  ( storages_provider.local_endpoint()
                                  )
                                // \todo accept transport_provider_connection_information
                              , transport_provider_connectable
                              , storage->id()
                              , mcs::core::storage::make_parameter (parameter_size_max)
                              , mcs::core::storage::make_parameter (parameter_size_used)
                              , mcs::core::storage::make_parameter (parameter_segment_create)
                              , mcs::core::storage::make_parameter (parameter_segment_remove)
                              , mcs::core::storage::make_parameter (parameter_chunk_description)
                              , mcs::core::storage::make_parameter (parameter_file_read)
                              , mcs::core::storage::make_parameter (parameter_file_write)
                              }
                            )
                          };

                        fmt::print ("storage_id: {}\n", storage_id);
                      }
                    );
                }

                fmt::print ( "storages_provider: {}\n"
                           , mcs::util::ASIO::make_connectable
                               (storages_provider.local_endpoint())
                           );
                fmt::print ( "transport_provider: {}\n"
                           , transport_provider_connectable
                           );

                // run "for ever"
                io_context_storages_provider.join();
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
