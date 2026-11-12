// Copyright (C) 2023-2026 Fraunhofer ITWM
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
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto share_service_path {Parameter {"share-service-path"}};
    constexpr auto size {Parameter {"size"}};
    constexpr auto storage {Parameter {"storage"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <PATH>\n"
        "    Directory containing share-service information.\n"
        "  --{2} <SIZE_IN_BYTES>\n"
        "    Size of the chunk to create.\n"
        "  --{3} <STORAGE_SPEC>\n"
        "    Files or SHMEM backend with creation parameters.\n"
      , args.at (0)
      , option::share_service_path
      , option::size
      , option::storage
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}\n", usage (args));
  }

  template< mcs::share_service::is_supported_storage_implementation
              Implementation
          >
    struct Create
  {
    using type = Implementation::Parameter::Create;
  };

  auto create_main (mcs::util::arguments::View const& args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::share_service_path)
        . add (option::size)
        . add (option::storage)
        )
      };
    auto const size
      { mcs::util::read::read<mcs::core::memory::Size>
          ( options.at (option::size)
          )
      };
    auto const share_service_path
      { std::filesystem::path {options.at (option::share_service_path)}
      };
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
              ( options.at (option::storage)
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
  return mcs::util::main (argc, argv, help, create_main);
}
