// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <mcs/block_device/Block.hpp>
#include <mcs/block_device/Reader.hpp>
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
#include <mcs/util/cast.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <mcs/util/type/List.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace
{
  auto cat_main (mcs::util::Args args) -> int
  {
    if (args.size() != 3)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} meta_data_provider_path block_id"
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
                { io_context
                , meta_data_provider
                }
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

          auto reader
            { mcs::block_device::Reader
              { meta_data_client
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
              }
            };

          auto const bytes_read {reader (block_id, block.address()).get()};

          if (bytes_read != size)
          {
            throw std::runtime_error {"Could not read all data."};
          }

          auto const chunk
            {block.template chunk<mcs::core::chunk::access::Const>()};
          auto const bytes {chunk.data()};

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
  return mcs::util::main (argc, argv, cat_main);
}
