// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/ASIO/Provider.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/ASIO/Endpoint.hpp>
#include <mcs/util/ASIO/is_protocol.hpp>
#include <mcs/util/TemporaryDirectory.hpp>
#include <mcs/util/TemporaryFile.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace
{
  auto provider_main (mcs::util::Args args) -> int
  {
    if (args.size() != 5)
    {
      throw std::invalid_argument
        { fmt::format
          ( "usage: {} endpoint provider_path number_of_longs number_of_threads"
          , args[0]
          )
        };
    }

    using Element = long;

    // prepare some data to provide
    auto const number_of_elements
      { mcs::util::read::read<unsigned int> (args[3])
      };
    auto const size
      {mcs::core::memory::make_size (number_of_elements * sizeof (Element))};


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
            { storages
              . template chunk_description
                  < Storage
                  , mcs::core::chunk::access::Mutable
                  >
                ( storages.read_access()
                , storage->id()
                , typename Storage::Parameter::Chunk::Description{}
                , segment->id()
                , mcs::core::memory::make_range
                    ( mcs::core::memory::make_offset (0)
                    , size
                    )
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
            { mcs::util::read::read<unsigned int> (args[4])
            }
        , SIGINT, SIGTERM
        }
      };

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::Endpoint> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
           (typename Protocol::endpoint provider_endpoint)
        {
          using TransportProvider
            = mcs::core::transport::implementation::ASIO::Provider
                < Protocol
                , SupportedStorageImplementations
                >
            ;

          auto const provider
            { TransportProvider
                { io_context
                , provider_endpoint
                , std::addressof (storages)
                }
            };

          // publish information for clients
          auto const temporary_directory
            { mcs::util::TemporaryDirectory {args[2]}
            };

          auto const address_information
            { mcs::util::TemporaryFile
              { temporary_directory.path() / "SOURCE"
              , "{}"
              , mcs::core::transport::Address
                  { storage->id()
                  , mcs::core::storage::make_parameter
                      (typename Storage::Parameter::Chunk::Description{})
                  , segment->id()
                  , mcs::core::memory::make_offset (0)
                  }
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

          // run "for ever"
          io_context.join();

          // \todo daemonize and proper return once the connection
          // information has been written

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, provider_main);
}
