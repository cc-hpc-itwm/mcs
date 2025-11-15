// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <csignal>
#include <mcs/rpc/Concepts.hpp>
#include <mcs/rpc/ScopedRunningIOContext.hpp>
#include <mcs/rpc/access_policy/Exclusive.hpp>
#include <mcs/share_service/Client.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mcs::share_service::bin
{
  template<typename Fun>
    auto with_chunk_client
      ( share_service::Chunk const& chunk
      , Fun const& fun
      )
  {
    return util::ASIO::run
      ( chunk.provider_connectable
      , [&]<util::ASIO::is_protocol Protocol>
          (util::ASIO::Connectable<Protocol> connectable)
        {
          return share_service::SupportedStorageImplementations::run
            ( chunk.storage_implementation_id
            , [&]< share_service::is_supported_storage_implementation
                     StorageImplementation
                 >
                (
                ) -> int
              {
                auto io_context
                  { rpc::ScopedRunningIOContext
                    { rpc::ScopedRunningIOContext::NumberOfThreads {1u}
                    , SIGINT, SIGTERM
                    }
                  };

                auto const client
                  { share_service::Client< Protocol
                                         , rpc::access_policy::Exclusive
                                         >
                    ( io_context
                    , connectable
                    )
                  };

                return fun.template operator()<StorageImplementation>
                  ( chunk
                  , client
                  );
              }
            );
        }
      );
  }
}
