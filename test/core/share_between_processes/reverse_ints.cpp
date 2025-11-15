// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "SupportedStorageImplementations.hpp"
#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <fmt/format.h>
#include <iterator>
#include <mcs/core/Chunk.hpp>
#include <mcs/core/control/Client.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto reverse_ints_main (mcs::util::Args args) -> int
  {
    if (args.size() != 6)
    {
      throw std::invalid_argument
        { fmt::format
            ( "usage: {} provider_connectable storage_id segment_id number_of_bytes parameter_chunk_description"
            , args[0]
            )
        };
    }

    auto const provider_connectable
      {mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])};
    auto const storage_id
      {mcs::util::read::read<mcs::core::storage::ID> (args[2])};
    auto const segment_id
      {mcs::util::read::read<mcs::core::storage::segment::ID> (args[3])};
    auto const number_of_bytes
      {mcs::util::read::read<mcs::core::memory::Size> (args[4])};
    auto const parameter_chunk_description
      {mcs::util::read::read<mcs::core::storage::Parameter> (args[5])};

    // Create a client for the remote provider and use the "local"
    // version of chunk_description()
    //
    auto io_context
      { mcs::rpc::ScopedRunningIOContext
          { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
          , SIGINT, SIGTERM
          }
      };

    return mcs::util::ASIO::run
      ( provider_connectable
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          (mcs::util::ASIO::Connectable<Protocol> connectable)
        {
          auto const client
            { mcs::core::control::Client
                  < Protocol
                  , mcs::rpc::access_policy::Exclusive
                  , SupportedStorageImplementations
                  >
                { io_context
                , std::move (connectable)
                }
            };

          auto const chunk
            { SupportedStorageImplementations::template wrap
                < mcs::core::Chunk
                , mcs::core::chunk::access::Mutable
                >
              { client
              . template chunk_description<mcs::core::chunk::access::Mutable>
                ( storage_id
                , parameter_chunk_description
                , segment_id
                , mcs::core::memory::make_range
                    ( mcs::core::memory::make_offset (0)
                    , number_of_bytes
                    )
                )
              }
            };

          auto const ints {as<int> (chunk)};
          std::ranges::reverse (ints);

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, reverse_ints_main);
}
