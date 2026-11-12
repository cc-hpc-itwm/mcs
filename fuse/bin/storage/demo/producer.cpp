// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

// This file represents a legacy application that writes data to
// files. It is intentionally kept simple and uses only standard POSIX
// I/O. The producer cannot be modified to use MCS data access methods
// directly.
//
// Usage: file_producer --file FILE --size BYTES [--chunk-size BYTES]
//
// The producer writes data to the file in chunks. Each chunk contains
// a pattern that allows verifying the data integrity later.

#include "data.hpp"
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <functional>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/buffer/Bytes.hpp>
#include <mcs/util/command_line/Parsed.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/ftruncate.hpp>
#include <mcs/util/syscall/pwrite.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  namespace option
  {
    using mcs::util::command_line::option::Parameter;

    using Presence = Parameter::Presence;
    constexpr auto chunk_size
      { Parameter {"chunk-size", Presence::Optional{}}
      };
    constexpr auto file {Parameter {"file"}};
    constexpr auto size {Parameter {"size"}};
  }

  [[nodiscard]] auto usage (mcs::util::arguments::View args)
  {
    return fmt::format
      ( "usage: {0} [OPTIONS]\n"
        "Required options:\n"
        "  --{1} <PATH>\n"
        "    Output file to create.\n"
        "  --{2} <SIZE_IN_BYTES>\n"
        "    Size of the output file.\n"
        "Optional settings:\n"
        "  --{3} <SIZE_IN_BYTES>\n"
        "    Bytes per write (default: 4096).\n"
      , args.at (0)
      , option::file
      , option::size
      , option::chunk_size
      );
  }

  auto help (mcs::util::arguments::View args)
  {
    fmt::print ("{}", usage (args));
  }

  auto mcs_fuse_bin_storage_demo_producer
    ( mcs::util::arguments::View args
    ) -> int
  {
    auto const options
      { args.from (1UL).parse
        ( mcs::util::command_line::Schema{}
        . add (option::file)
        . add (option::size)
        . add (option::chunk_size)
        )
      };
    auto const file_path
      { std::filesystem::path {options.at (option::file)}
      };
    auto const file_size
      { mcs::util::read::read<std::size_t> (options.at (option::size))
      };
    auto const chunk_size
      { std::invoke
        ( [&]
          {
            if (auto const value {options.optional (option::chunk_size)})
            {
              return mcs::util::read::read<std::size_t> (*value);
            }

            return std::size_t {4 << 10};
          }
        )
      };
    if (chunk_size == 0)
    {
      throw std::invalid_argument
        { fmt::format ("--{0} must be positive", option::chunk_size)
        };
    }

    fmt::print
      ( "Writing {} bytes to {}\n"
      , file_size
      , mcs::util::FMT::STD::FILESYSTEM::Path {file_path}
      );

    auto const file {mcs::util::fopen (file_path, "wb")};
    auto const fd {mcs::util::syscall::fileno (file.get())};

    // Pre-allocate the file to its final size. This allows the
    // filesystem to allocate all required storage upfront.
    //
    mcs::util::syscall::ftruncate (fd, static_cast<off_t> (file_size));

    auto const buffer
      { mcs::util::buffer::Bytes {std::min (file_size, chunk_size)}
      };

    auto offset {std::size_t {0}};

    while (offset < file_size)
    {
      auto const bytes_to_write {std::min (buffer.size(), file_size - offset)};
      auto const chunk {buffer.data<std::byte>().first (bytes_to_write)};

      mcs::fuse::demo::generate (chunk, offset);

      auto const written
        { mcs::util::syscall::pwrite
          ( fd
          , chunk.data()
          , chunk.size()
          , static_cast<off_t> (offset)
          )
        };

      if (std::cmp_not_equal (written, bytes_to_write))
      {
        throw std::runtime_error
          { fmt::format
            ( "Write error at offset '{}': expected '{}' bytes, wrote '{}'"
            , offset
            , bytes_to_write
            , written
            )
          };
      }

      offset += bytes_to_write;
    }

    fmt::print
      ( "SUCCESS: Wrote {} bytes to {}.\n"
      , file_size
      , mcs::util::FMT::STD::FILESYSTEM::Path {file_path}
      );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main
    ( argc
    , argv
    , help
    , mcs_fuse_bin_storage_demo_producer
    );
}
