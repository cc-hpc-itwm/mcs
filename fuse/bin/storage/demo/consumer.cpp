// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

// This is the MCS-aware consumer that reads data from files on an MCS
// FUSE filesystem by directly accessing the underlying SHMEM storage.
//
// Instead of reading the file content through the FUSE layer, this
// consumer:
//   1. Uses an ioctl to query the file's distribution in the MCS storage
//   2. Opens the SHMEM segments directly
//   3. Accesses the data without any copy (zero-copy)
//
// This demonstrates how a consumer can benefit from MCS even when the
// producer is a legacy application that cannot be modified.
//
// Usage: mcs_consumer <shmem_prefix> <file>
//
// Prerequisites:
//   - The file must be on a FUSE filesystem backed by SHMEM storage
//   - The shmem_prefix must match the one used when starting the
//     filesystem

#include "data.hpp"
#include <cstddef>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/format.h>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/fuse/content/implementation/storage/Part.hpp>
#include <mcs/fuse/content/implementation/storage/for_each_part.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/string.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munmap.hpp>
#include <mcs/util/syscall/shm_open.hpp>
#include <span>

namespace
{
  struct MappedPart
  {
    [[nodiscard]] MappedPart (std::string name, std::size_t size)
      : _size {size}
      , _data
        { std::invoke
          ( [&]
            {
              auto const fd
                { mcs::util::syscall::shm_open (name.c_str(), O_RDONLY, 0)
                };

              auto const close_fd
                { mcs::nonstd::make_scope_exit_that_dies_on_exception
                  ( "MappedPart::close"
                  , [fd]
                    {
                      mcs::util::syscall::close (fd);
                    }
                  )
                };

              return mcs::util::syscall::mmap
                ( nullptr
                , _size
                , PROT_READ
                , MAP_SHARED
                , fd
                , 0
                );
            }
          )
        }
    {}

    ~MappedPart()
    {
      if (_data != nullptr)
      {
        mcs::util::syscall::munmap (_data, _size);
      }
    }

    MappedPart (MappedPart const&) = delete;
    auto operator= (MappedPart const&) -> MappedPart& = delete;
    MappedPart (MappedPart&&) = delete;
    auto operator= (MappedPart&&) -> MappedPart& = delete;

    [[nodiscard]] constexpr auto data
      (
      ) const noexcept -> std::span<std::byte const>
    {
      return {mcs::util::cast<std::byte const*> (_data), _size};
    }

  private:
    std::size_t _size;
    void* _data;
  };

  auto mcs_fuse_bin_storage_demo_consumer (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format
          ( "Usage: {} <shmem_prefix> <file>\n"
            "\n"
            "Reads data from a file on an MCS FUSE filesystem by directly\n"
            "accessing the underlying SHMEM storage (zero-copy).\n"
            "\n"
            "Arguments:\n"
            "  shmem_prefix  The prefix used when starting the FUSE filesystem\n"
            "  file          Path to a file on the MCS FUSE filesystem\n"
            "\n"
            "The consumer queries the file distribution using an ioctl,\n"
            "then opens each SHMEM segment directly to verify the data\n"
            "pattern written by the producer.\n"
          , args[0]
          )
       };
    }

    auto const shmem_prefix {mcs::util::string {args[1]}};
    auto const file_path {std::filesystem::path {args[2]}};

    fmt::print ( "Reading {} via SHMEM (prefix: {})\n"
               , file_path
               , shmem_prefix
               );

    // Verify each part of the file by directly accessing the data.
    //
    auto bytes_verified {std::size_t {0}};

    mcs::fuse::content::storage::for_each_part
      ( file_path
      , [&] (mcs::fuse::content::storage::Part const& file_part)
        {
          fmt::print ("  Found part: {}\n", file_part);

          using mcs::core::memory::begin;
          using mcs::core::memory::offset_cast;
          using mcs::core::memory::size;
          using mcs::core::memory::size_cast;

          auto const file_offset
            { offset_cast<std::size_t> (begin (file_part.range()))
            };
          auto const part_size
            { size_cast<std::size_t> (size (file_part.range()))
            };

          auto const name
            { fmt::format ("/{}.{}", shmem_prefix, file_part.segment_id())
            };
          auto const part {MappedPart {name, part_size}};

          mcs::fuse::demo::verify (part.data(), file_offset);

          bytes_verified += part_size;
        }
      );

    auto const file_size {std::filesystem::file_size (file_path)};

    if (file_size != bytes_verified)
    {
      throw std::runtime_error
        { fmt::format
            ( "File {} has size {}, and only {} many bytes could be verified."
            , file_path
            , file_size
            , bytes_verified
            )
        };
    }

    fmt::print ( "SUCCESS: Verified {} bytes of {}.\n"
               , bytes_verified
               , file_path
               );

    return EXIT_SUCCESS;
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, mcs_fuse_bin_storage_demo_consumer);
}
