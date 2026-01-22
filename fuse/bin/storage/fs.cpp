// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdlib>
#include <exception>
#include <fmt/base.h>
#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/fuse/content/implementation/Storage.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/main.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/type/List.hpp>

namespace
{
  using SupportedStorageImplementations = mcs::util::type::List
    < mcs::core::storage::implementation::Files
    , mcs::core::storage::implementation::Heap
    , mcs::core::storage::implementation::SHMEM
    >
    ;

  template<typename I>
    concept is_supported_storage_implementation =
      SupportedStorageImplementations::template contains<I>()
    ;

  template<is_supported_storage_implementation Implementation>
    struct Create
  {
    using type = Implementation::Parameter::Create;
  };
}

auto main (int argc, char** argv) noexcept -> int
try
{
  auto const print_usage
    { [&]
      {
        mcs::util::FMT::print_noexcept
          ( stdout
          , "usage: {}"
          " STORAGE"                                       // 1
          " STORAGE_parameter_size_max"                    // 2
          " STORAGE_parameter_size_used"                   // 3
          " STORAGE_parameter_segment_create"              // 4
          " STORAGE_parameter_segment_remove"              // 5
          " STORAGE_parameter_chunk_description"           // 6
          " NAME"                                          // 7
          " [fuse_options] <mountpoint>\n\n"
          , argv[0]
          );

        ::fuse_cmdline_help();
        ::fuse_lowlevel_help();
      }
    };

  if (argc < 8)
  {
    print_usage();

    return EXIT_FAILURE;
  }

  return SupportedStorageImplementations::template parse_id_and_run<Create>
    ( argv[1]
    , [&] <is_supported_storage_implementation StorageImplementation>
        ( typename StorageImplementation::Parameter::Create parameter_create
        )
      {
        using Content
          = mcs::fuse::content::implementation::Storage<StorageImplementation>
          ;
        using Parameter = typename StorageImplementation::Parameter;
        using mcs::util::read::read;

        return mcs::fuse::main<Content>
          ( print_usage
          , argc - 7
          , argv + 7
          , typename Content::State
            { parameter_create
            , read<typename Parameter::Size::Max> (argv[2])
            , read<typename Parameter::Size::Used> (argv[3])
            , read<typename Parameter::Segment::Create> (argv[4])
            , read<typename Parameter::Segment::Remove> (argv[5])
            , read<typename Parameter::Chunk::Description> (argv[6])
            }
          );
      }
    );
}
catch (...)
{
  mcs::util::FMT::print_noexcept
    ( stderr
    , "Error: {}\n"
    , std::current_exception()
    );

   return EXIT_FAILURE;
}
