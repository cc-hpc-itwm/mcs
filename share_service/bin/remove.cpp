// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "with_chunk_client.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto remove_main (mcs::util::Args const& args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument {fmt::format ("usage: {} chunk\n", args[0])};
    }

    return mcs::share_service::bin::with_chunk_client
      ( mcs::util::read::read<mcs::share_service::Chunk> (args[1])
        , [&]< mcs::share_service::is_supported_storage_implementation
                 StorageImplementation
             >
          ( mcs::share_service::Chunk const& chunk
          , auto& client
          )
        {
          namespace COMMAND = mcs::share_service::command;

          client.remove
            ( chunk
            , typename COMMAND::remove::Parameters<StorageImplementation>{}
            );

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, remove_main);
}
