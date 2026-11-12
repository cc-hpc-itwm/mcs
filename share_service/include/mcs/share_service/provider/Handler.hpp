// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/share_service/command/Attach.hpp>
#include <mcs/share_service/command/Create.hpp>
#include <mcs/share_service/command/Remove.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::share_service::provider
{
  template<typename StorageImplementations> struct Handler;

  template<is_supported_storage_implementation... StorageImplementations>
    struct Handler<util::type::List<StorageImplementations...>>
  {
    using Storages
      = core::Storages<util::type::List<StorageImplementations...>>
      ;

    explicit Handler (util::not_null<Storages>);

    auto operator() (command::Create) -> command::Create::Response;
    auto operator() (command::Remove) -> command::Remove::Response;

    template<core::chunk::is_access Access>
      auto operator()
        ( command::Attach<Access>
        ) -> typename command::Attach<Access>::Response
      ;

  private:
    util::not_null<Storages> _storages;
  };
}

#include "detail/Handler.ipp"
