// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "with_chunk_client.hpp"
#include <algorithm>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <iterator>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/copy_at_most_n.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto write_main (mcs::util::Args const& args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        {fmt::format ("usage: echo 'DATA' | {} chunk\n", args[0])};
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

          auto const data
            { client.template attach<mcs::core::chunk::access::Mutable>
                ( chunk
                , typename COMMAND::attach::Parameters<StorageImplementation>{}
                )
            };

          mcs::util::copy_at_most_n
            ( std::istream_iterator<char> {std::cin >> std::noskipws}
            , std::istream_iterator<char>{}
            , mcs::core::memory::size_cast<std::size_t> (chunk.size)
            , std::begin (as<char> (data))
            );

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, write_main);
}
