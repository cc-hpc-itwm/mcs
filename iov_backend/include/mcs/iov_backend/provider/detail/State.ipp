// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/nonstd/scope.hpp>
#include <stdexcept>

namespace mcs::iov_backend::provider
{
  template<typename Executor>
    auto State::restart
      ( Executor& io_context
      , util::not_null<StoragesClients> storages_clients
      ) -> State&
  {
    storages_clients->restart (io_context, _storages, _collections);

    return *this;
  }

  template<typename Executor>
    auto State::add
      ( Executor& io_context
      , util::not_null<StoragesClients> storages_clients
      , Storage storage
      ) -> storage::ID
  {
    auto const increment_next_storage_id
      { nonstd::make_scope_exit
          ( [&]() noexcept
            {
              ++_next_storage_id;
            }
          )
      };

    // \note state change repeated in stateful client
    auto const [pos, inserted] {_storages.emplace (_next_storage_id, storage)};

    if (!inserted)
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
            ( "mcs::iov_backend::provider::State: Duplicate storage::ID: '{}'"
            , _next_storage_id
            )
        };
    }

    return storages_clients->add (io_context, pos->first, storage);
  }
}
