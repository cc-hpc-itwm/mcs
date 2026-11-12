// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "with_chunk_client.hpp"
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <mcs/core/chunk/access/Const.hpp>
#include <mcs/share_service/Chunk.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    constexpr auto chunk {Parameter {"chunk"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <CHUNK>\n"
        "    Serialized descriptor of the chunk to read.\n"
      , args.at (0)
      , option::chunk
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}", usage (args));
  }

  auto cat_main (mcs::util::arguments::View const& args) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::chunk)
        )
      };
    return mcs::share_service::bin::with_chunk_client
      ( mcs::util::read::read<mcs::share_service::Chunk>
          ( options.at (option::chunk)
          )
        , [&]< mcs::share_service::is_supported_storage_implementation
                 StorageImplementation
             >
          ( mcs::share_service::Chunk const& chunk
          , auto& client
          )
        {
          namespace COMMAND = mcs::share_service::command;

          auto const data
            { client.template attach<mcs::core::chunk::access::Const>
                ( chunk
                , typename COMMAND::attach::Parameters<StorageImplementation>{}
                )
            };
          auto const bytes {data.data()};

          if ( std::cout.write
                 ( mcs::util::cast<char const*> (bytes.data())
                 , mcs::util::cast<std::streamsize> (bytes.size())
                 ).fail()
             )
          {
            throw std::runtime_error {"Failure, broken pipe."};
          }

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, help, cat_main);
}
