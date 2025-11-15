// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cassert>
#include <exception>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iterator>
#include <mcs/core/control/command/segment/Remove.hpp>
#include <mcs/iov_backend/UsedStorage.hpp>
#include <mcs/iov_backend/provider/StoragesClients.hpp>
#include <mcs/rpc/multi_client/ParallelCallsLimit.hpp>
#include <mcs/rpc/multi_client/call_and_collect.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <memory>
#include <unordered_map>

namespace mcs::iov_backend::provider
{
  auto StoragesClients::at
    ( storage::ID storage_id
    ) -> AnyClient&
  {
    if ( auto pos {_clients_by_storage_id.find (storage_id)}
       ; pos != std::end (_clients_by_storage_id)
       )
    {
      return pos->second;
    }

    // \todo specific exception
    throw std::invalid_argument
      { fmt::format
          ( "mcs::iov_backend::provider::StoragesClients"
            ": Unknown storage::ID: '{}'"
          , storage_id
          )
      };
  }

  auto StoragesClients::remove
    ( state::Storages const& storages
    , UsedStorages const& used_storages
    ) -> void
  {
    using SegmentRemove = core::control::command::segment::Remove;

    auto errors {std::unordered_map<storage::ID, std::exception_ptr>{}};

    struct Collector
    {
      auto result ( UsedStorage const&
                  , SegmentRemove::Response
                  ) noexcept
      {
        // ignore
      }
      auto error ( UsedStorage const& used_storage
                 , std::exception_ptr error
                 ) noexcept
      {
        _errors->emplace (used_storage._storage_id, error);
      }

      std::unordered_map<storage::ID, std::exception_ptr>* _errors;
    };
    auto collector {Collector {std::addressof (errors)}};

    rpc::multi_client::call_and_collect<SegmentRemove>
      ( [&] (auto const& used_storage)
        {
          auto const& storage {storages.at (used_storage._storage_id)};

          return SegmentRemove
            { storage._storage_id
            , storage._parameter_segment_remove
            , used_storage._segment_id
            };
        }
      , [&] (auto const& used_storage)
        {
          return at (used_storage._storage_id);
        }
      , collector
      , used_storages
        // \todo parameter for the ParallelCallsLimit
      , rpc::multi_client::ParallelCalls::Unlimited{}
      );

    if (!errors.empty())
    {
      throw std::runtime_error
        { fmt::format ("StoragesClients::remove: {}", errors)
        };
    }
  }
}
