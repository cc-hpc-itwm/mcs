// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <iterator>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/libfabric/Provider.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Domain.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Interface.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Provider.hpp>
#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Provider.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/daemon.hpp>
#include <mcs/util/daemon/option.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto control_endpoint {Parameter {"control-endpoint"}};
    constexpr auto number_of_longs {Parameter {"number-of-longs"}};
    constexpr auto number_of_threads {Parameter {"number-of-threads"}};
    constexpr auto provider_path {Parameter {"provider-path"}};
    constexpr auto transport_domain_name
      { Parameter {"transport-domain-name"}
      };
    constexpr auto transport_provider_name
      { Parameter {"transport-provider-name"}
      };
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <ENDPOINT>\n"
        "    RPC control listening endpoint.\n"
        "  --{2} <NAME>\n"
        "    Libfabric provider name.\n"
        "  --{3} <NAME>\n"
        "    Libfabric domain name.\n"
        "  --{4} <PATH>\n"
        "    Directory for publishing provider and source information.\n"
        "  --{5} <COUNT>\n"
        "    Number of long elements to provide.\n"
        "  --{6} <COUNT>\n"
        "    Number of RPC I/O threads.\n"
      , args.at (0)
      , option::control_endpoint
      , option::transport_provider_name
      , option::transport_domain_name
      , option::provider_path
      , option::number_of_longs
      , option::number_of_threads
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print
      ( "{0}\nOptional settings:\n"
        "  {1}  Run in foreground (default: background)\n"
      , usage (args)
      , mcs::util::daemon::option::foreground
      );
  }

  auto provider_main
    ( mcs::util::arguments::View args
    , mcs::util::Daemon& daemon
    ) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::control_endpoint)
        . add (option::transport_provider_name)
        . add (option::transport_domain_name)
        . add (option::provider_path)
        . add (option::number_of_longs)
        . add (option::number_of_threads)
        )
      };
    using Element = long;

    // prepare some data to provide
    auto const number_of_elements
      { mcs::util::read::read<unsigned int>
          ( options.at (option::number_of_longs)
          )
      };
    auto const size
      { mcs::core::memory::make_size (number_of_elements * sizeof (Element))
      };

    using Storage = mcs::core::storage::implementation::Heap;
    using SupportedStorageImplementations = mcs::util::type::List<Storage>;

    auto storages {mcs::core::Storages<SupportedStorageImplementations>{}};
    auto const storage
      { mcs::core::make_unique_storage<Storage>
          ( std::addressof (storages)
          , Storage::Parameter::Create
              { mcs::core::storage::MaxSize::Limit {size}
              }
          )
      };
    auto const segment
      { mcs::core::storage::make_unique_segment<Storage>
          ( std::addressof (storages)
          , storage->id()
          , size
          )
      };

    {
      auto chunk
        { SupportedStorageImplementations::template wrap
              < mcs::core::Chunk
              , mcs::core::chunk::access::Mutable
              >
            { storages.read_access().template invoke<Storage>
              ( storage->id()
              , [&] (auto const& storage_implementation)
                {
                  return storage_implementation
                    . template chunk_description
                        < mcs::core::chunk::access::Mutable
                        >
                      ( typename Storage::Parameter::Chunk::Description{}
                      , segment->id()
                      , mcs::core::memory::make_range
                          ( mcs::core::memory::make_offset (0)
                          , size
                          )
                      )
                    ;
                }
              )
            }
        };
      auto elements {mcs::core::as<Element> (chunk)};
      std::iota (std::begin (elements), std::end (elements), 0l);
    }

    // create a provider
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads
            { mcs::util::read::read<unsigned int>
                ( options.at (option::number_of_threads)
                )
            }
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint>
          ( options.at (option::control_endpoint)
          )
      , [&]<mcs::util::ASIO::is_protocol Protocol>
           (typename Protocol::endpoint endpoint)
        {
          namespace libfabric
            = mcs::core::transport::implementation::libfabric
            ;

          using Transporter = libfabric::transporter::rdm::Provider;
          using Provider
            = libfabric::Provider
                < Protocol
                , Transporter
                , SupportedStorageImplementations
                >
            ;

          auto provider
            { Provider
                { io_context
                , endpoint
                , libfabric::libfabric::Interface
                    { .provider = libfabric::libfabric::Provider
                        { std::string
                            {options.at (option::transport_provider_name)}
                        }
                    , .domain = libfabric::libfabric::Domain
                        { std::string
                            {options.at (option::transport_domain_name)}
                        }
                    }
                , std::addressof (storages)
                }
            };

          // publish information for clients
          auto const temporary_directory
            { mcs::util::TemporaryDirectory
              { options.at (option::provider_path)
              }
            };

          auto const address_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "SOURCE"
              , "{}"
                , mcs::core::transport::make_address
                  ( storage->id()
                  , mcs::core::storage::make_parameter
                      (typename Storage::Parameter::Chunk::Description{})
                  , segment->id()
                  , mcs::core::memory::make_offset (0)
                  )
              }
            };

          auto const provider_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "PROVIDER"
              , "{}"
              , provider.connection_information()
              }
            };

          auto const pid_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "PID"
              , "{}"
              , mcs::util::syscall::getpid()
              }
            };

          daemon.ready();

          io_context.join();

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::daemonize (argc, argv, help, provider_main);
}
