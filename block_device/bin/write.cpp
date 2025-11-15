// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <ios>
#include <iostream>
#include <iterator>
#include <mcs/block_device/Block.hpp>
#include <mcs/block_device/Writer.hpp>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/block_device/meta_data/Client.hpp>
#include <mcs/core/Storages.hpp>
#include <mcs/core/UniqueStorage.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/UniqueSegment.hpp>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/core/transport/implementation/ASIO/Client.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/copy_at_most_n.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace
{
  auto write_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format
          ( "usage: echo 'DATA' | {} meta_data_provider_path block_id"
          , args[0]
          )
        };
    }

    auto const meta_data_provider_path {std::filesystem::path (args[1])};
    auto const meta_data_provider_connectable
      { mcs::util::read::from_file<mcs::util::ASIO::AnyConnectable>
          (meta_data_provider_path / "PROVIDER")
      };

    auto const block_id
      {mcs::util::read::read<mcs::block_device::block::ID> (args[2])};

    auto io_context
      { mcs::rpc::ScopedRunningIOContext
        { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
        , SIGINT, SIGTERM
        }
      };

    using Storage = mcs::core::storage::implementation::Heap;
    using SupportedClientStorageImplementations
      = mcs::util::type::List<Storage>
      ;
    auto storages
      { mcs::core::Storages<SupportedClientStorageImplementations>{}
      };

    return mcs::util::ASIO::run
      ( meta_data_provider_connectable
      , [&]<mcs::util::ASIO::is_protocol MetaDataProtocol>
          (mcs::util::ASIO::Connectable<MetaDataProtocol> meta_data_provider)
        {
          auto meta_data_client
            { mcs::block_device::meta_data::Client
                  < MetaDataProtocol
                  , mcs::rpc::access_policy::Exclusive
                  >
                ( io_context
                , meta_data_provider
                )
            };

          auto const size
            {mcs::core::memory::make_size (meta_data_client.block_size())};

          auto const storage
            { mcs::core::make_unique_storage<Storage>
                ( std::addressof (storages)
                , Storage::Parameter::Create
                    { mcs::core::storage::MaxSize
                      { mcs::core::storage::MaxSize::Limit {size}
                      }
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
          auto const block
            { mcs::block_device::Block<SupportedClientStorageImplementations>
              { std::addressof (storages)
              , storage->id()
              , mcs::core::storage::make_parameter
                ( Storage::Parameter::Chunk::Description{}
                )
              , segment->id()
              , mcs::core::memory::make_range
                  (mcs::core::memory::make_offset (0), size)
              }
            };

          auto const chunk
            {block.template chunk<mcs::core::chunk::access::Mutable>()};
          auto const data {std::begin (as<char> (chunk))};

          auto const bytes_copied
            { std::distance
              ( data
              , mcs::util::copy_at_most_n
                ( std::istream_iterator<char> {std::cin >> std::noskipws}
                , std::istream_iterator<char>{}
                , mcs::core::memory::size_cast<std::size_t> (size)
                , data
                )
              )
            };

          if (mcs::core::memory::make_size (bytes_copied) != size)
          {
            fmt::print
              ( stderr
              , "Warning: Not enough data provided to fill the complete block"
                ": block size is '{}' and input contains '{}' bytes only.\n"
              , size
              , bytes_copied
              );
          }

          if (  std::istream_iterator<char> {std::cin}
             != std::istream_iterator<char>{}
             )
          {
            fmt::print
              ( stderr
              , "Warning: Too much data provided"
                ": block size is '{}' and input contains more bytes.\n"
              , size
              );
          }

          auto writer
            { mcs::block_device::Writer
                ( meta_data_client
                , [&]<mcs::util::ASIO::is_protocol TransportProtocol>
                    ( mcs::util::ASIO::Connectable<TransportProtocol>
                        storages_provider
                    )
                  {
                    return mcs::core::transport::implementation::ASIO::Client
                        < TransportProtocol
                        , mcs::rpc::access_policy::Exclusive
                        , SupportedClientStorageImplementations
                        >
                      { io_context
                      , storages_provider
                      , std::addressof (storages)
                      };
                  }
                )
            };

          auto const bytes_written {writer (block_id, block.address()).get()};

          if (bytes_written != size)
          {
            throw std::runtime_error {"Could not write all data."};
          }

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, write_main);
}
