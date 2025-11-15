// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <mcs/core/memory/Size.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/share_service/Client.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  template< mcs::share_service::is_supported_storage_implementation
              Implementation
          >
    struct Create
  {
    using type = Implementation::Parameter::Create;
  };

  auto create_main (mcs::util::Args const& args) -> int
  {
    auto const usage
      { fmt::format
        ( "usage: {} share_service_path size STORAGE\n"
          "\n"
          "Only STORAGEs 'Files' and 'SHMEM' are supported."
        , args[0]
        )
        // \todo collect parse information from SupportedImplementations
      };

    if (args.size() != 4)
    {
      throw std::invalid_argument {usage};
    }

    auto const size {mcs::util::read::read<mcs::core::memory::Size> (args[2])};
    auto const share_service_path {std::filesystem::path (args[1])};
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
          { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (share_service_path / "PROVIDER")
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (mcs::util::ASIO::Connectable<Protocol> connectable)
        {
          auto const client
            { mcs::share_service::Client< Protocol
                                        , mcs::rpc::access_policy::Exclusive
                                        >
              ( io_context
              , connectable
              )
            };

          auto const chunk
            { mcs::share_service::SupportedStorageImplementations
                ::template parse_id_and_run<Create>
              ( args[3]
              , [&]<mcs::share_service::is_supported_storage_implementation
                      StorageImplementation
                   >
                  ( typename Create<StorageImplementation>::type create
                  ) -> mcs::share_service::Chunk
                {
                  namespace COMMAND = mcs::share_service::command;
                  using Parameter = typename StorageImplementation::Parameter;
                  return client.template create<StorageImplementation>
                    ( size
                    , COMMAND::create::Parameters<StorageImplementation>
                        { create
                        , typename Parameter::Segment::Create{}
                        }
                    );
                }
              )
            };

          fmt::print ("{}\n", chunk);

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, create_main);
}
