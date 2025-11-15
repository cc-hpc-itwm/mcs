// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <cstddef>
#include <fmt/format.h>
#include <mcs/iov_backend/Client.hpp>
#include <mcs/iov_backend/SupportedStorageImplementations.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/main.hpp>
#include <mcs/util/read/read.hpp>
#include <stdexcept>

namespace
{
  auto list_storages_main (mcs::util::Args args) -> int
  {
    if (args.size() != 2)
    {
      throw std::invalid_argument
        { fmt::format ( "usage: {} provider"
                      , args[0]
                      )
        };
    }

    return mcs::util::ASIO::run
      ( mcs::util::read::read<mcs::util::ASIO::AnyConnectable> (args[1])
      , [&]<mcs::util::ASIO::is_protocol Protocol>
          ( mcs::util::ASIO::Connectable<Protocol> provider_connectable
          )
        {
          auto io_context
            { mcs::rpc::ScopedRunningIOContext
              { mcs::rpc::ScopedRunningIOContext::NumberOfThreads {1u}
              , SIGINT, SIGTERM
              }
            };

          auto const client
            { mcs::iov_backend::Client
                < Protocol
                , mcs::rpc::access_policy::Exclusive
                >
              { io_context
              , provider_connectable
              }
            };

          for (auto const& [storage_id, storage] : client.provider_state().storages())
          {
            mcs::iov_backend::SupportedStorageImplementations::run
              ( storage._storage_implementation_id
              , [&]< mcs::iov_backend::is_supported_storage_implementation
                       StorageImplementation
                   >()
                {
                  using Parameter = typename StorageImplementation::Parameter;
                  fmt::print ( "id: {}"
                             ", tag: {}"
                             ", parameter_create: {}"
                             ", storage_provider: {}/{}"
                             ", transport_provider: {}"
                             ", parameter_segment_create: {}"
                             ", parameter_segment_remove: {}"
                             ", parameter_chunk_description: {}"
                             "\n"
                             , storage_id
                             , typename StorageImplementation::Tag{}
                             , storage._parameter_create
                               .template as<typename Parameter::Create>()
                             , storage._storages_provider
                             , storage._storage_id
                             , storage._transport_provider
                             , storage._parameter_segment_create
                               .template as<typename Parameter::Segment::Create>()
                             , storage._parameter_segment_remove
                               .template as<typename Parameter::Segment::Remove>()
                             , storage._parameter_chunk_description
                               .template as<typename Parameter::Chunk::Description>()
                             );
                }
              );
          }

          return EXIT_SUCCESS;
        }
      );
  }
}

auto main (int argc, char const** argv) noexcept -> int
{
  return mcs::util::main (argc, argv, list_storages_main);
}
