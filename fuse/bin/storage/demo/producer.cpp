// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

// This file represents a legacy application that writes data to
// files. It is intentionally kept simple and uses only standard POSIX
// I/O. The producer cannot be modified to use MCS data access methods
// directly.
//
// Usage: file_producer <file> <size_in_bytes> [<chunk_size>]
//
// The producer writes data to the file in chunks. Each chunk contains
// a pattern that allows verifying the data integrity later.

#include "data.hpp"
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/buffer/Bytes.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/ftruncate.hpp>
#include <mcs/util/syscall/pwrite.hpp>
#include <stdexcept>
#include <utility>

namespace
{
  auto mcs_fuse_bin_storage_demo_producer (mcs::util::Args args) -> int
  {
    if (args.size() < 3 || args.size() > 4)
    {
      throw std::invalid_argument
        { fmt::format
          ( "Usage: {} <file> <size_in_bytes> [<chunk_size>]\n"
            "\n"
            "Writes data with a reproducible pattern to <file>.\n"
            "The pattern can be verified by a consumer application.\n"
            "\n"
            "  chunk_size  Write chunk size in bytes (default: 4096)\n"
          , args[0]
          )
        };
    }

    auto const file_path {std::filesystem::path {args[1]}};
    auto const file_size {mcs::util::read::read<std::size_t> (args[2])};

    fmt::print ("Writing {} bytes to {}\n", file_size, file_path);

    auto const file {mcs::util::fopen (file_path, "wb")};
    auto const fd {mcs::util::syscall::fileno (file.get())};

    // Pre-allocate the file to its final size. This allows the
    // filesystem to allocate all required storage upfront.
    //
    mcs::util::syscall::ftruncate (fd, static_cast<off_t> (file_size));

    auto const chunk_size
      { args.size() > 3
      ? mcs::util::read::read<std::size_t> (args[3])
      : std::size_t {4 << 10}
      };
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

    fmt::print ("SUCCESS: Wrote {} bytes to {}.\n", file_size, file_path);

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, mcs_fuse_bin_storage_demo_producer);
}
