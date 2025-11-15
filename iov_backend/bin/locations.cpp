// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fmt/format.h>
#include <mcs/core/memory/Range.hpp>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto locations_main (mcs::util::Args args) -> int
  {
    if (args.size() != 4)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} provider collection_id range"
                      , args[0]
                      )
        };
    }

    mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          ( mcs::util::ASIO::Connectable<Protocol> provider_connectable
          )
        {
          auto io_context
            { mcs::rpc::ScopedRunningIOContext
              { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
              , SIGINT, SIGTERM
              }
            };

          auto const client
            { mcs::iov_backend::Client
                < Protocol
                , mcs::rpc::access_policy::Exclusive
                >
              { io_context
              , provider_connectable
              }
            };

          for ( auto const& [ range
                            , storages_provider
                            , storage_implementation_id
                            , parameter_file_read
                            , parameter_file_write
                            , transport_provider
                            , address
                            ]
              : client.locations
                ( mcs::iov_backend::collection::ID {args[2]}
                , mcs::util::read::read<mcs::core::memory::Range> (args[3])
                )
              )
          {
            mcs::iov_backend::SupportedStorageImplementations::run
              ( storage_implementation_id
              , [&]< mcs::iov_backend::is_supported_storage_implementation
                       StorageImplementation
                   >()
                {
                  using Parameter = StorageImplementation::Parameter;
                  fmt::print
                    ( "{}: {}/{} via {}/({}, {})\n"
                    , range
                    , transport_provider
                    , address
                    , storages_provider
                    , parameter_file_read
                      .template as<typename Parameter::File::Read>()
                    , parameter_file_write
                      .template as<typename Parameter::File::Write>()
                    );
                }
              );
          }
        }
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, locations_main);
}
