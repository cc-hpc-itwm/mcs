// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Storages.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/control/command/chunk/Description.hpp>
#include <mcs/core/control/command/file/Read.hpp>
#include <mcs/core/control/command/file/Write.hpp>
#include <mcs/core/control/command/segment/Create.hpp>
#include <mcs/core/control/command/segment/Remove.hpp>
#include <mcs/core/control/command/storage/Create.hpp>
#include <mcs/core/control/command/storage/Remove.hpp>
#include <mcs/core/control/command/storage/size/Max.hpp>
#include <mcs/core/control/command/storage/size/Used.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::control::provider
{
  template<storage::is_implementation... StorageImplementations>
    struct Handler
  {
    explicit Handler
      ( util::not_null<Storages<util::type::List<StorageImplementations...>>>
      ) noexcept;

    template<chunk::is_access Access>
      auto operator()
        ( command::chunk::Description<Access, StorageImplementations...>
        ) const -> typename command::chunk::
                     Description<Access, StorageImplementations...>::Response
      ;
    auto operator()
      ( command::file::Read
      ) const -> command::file::Read::Response
      ;
    auto operator()
      ( command::file::Write
      ) const -> command::file::Write::Response
      ;
    auto operator()
      ( command::segment::Create
      ) -> command::segment::Create::Response
      ;
    auto operator()
      ( command::segment::Remove
      ) -> command::segment::Remove::Response
      ;
    auto operator()
      ( command::storage::Create<StorageImplementations...>
      ) -> typename command::storage::
             Create<StorageImplementations...>::Response
      ;
    auto operator()
      ( command::storage::Remove
      ) -> command::storage::Remove::Response
      ;
    auto operator()
      ( command::storage::Size
      ) -> command::storage::Size::Response
      ;
    auto operator()
      ( command::storage::size::Max
      ) const -> command::storage::size::Max::Response
      ;
    auto operator()
      ( command::storage::size::Used
      ) const -> command::storage::size::Used::Response
      ;

  private:
    util::not_null<Storages<util::type::List<StorageImplementations...>>>
      _storages;
  };
}

#include "detail/Handler.ipp"
