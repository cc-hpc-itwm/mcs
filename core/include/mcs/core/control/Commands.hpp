// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/control/command/chunk/Description.hpp>
#include <mcs/core/control/command/file/Read.hpp>
#include <mcs/core/control/command/file/Write.hpp>
#include <mcs/core/control/command/segment/Create.hpp>
#include <mcs/core/control/command/segment/Remove.hpp>
#include <mcs/core/control/command/storage/Create.hpp>
#include <mcs/core/control/command/storage/Remove.hpp>
#include <mcs/core/control/command/storage/Size.hpp>
#include <mcs/core/control/command/storage/size/Max.hpp>
#include <mcs/core/control/command/storage/size/Used.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::control::provider
{
  template<storage::is_implementation... StorageImplementations>
    using Commands = util::type::List
      < command::chunk::Description< chunk::access::Const
                                   , StorageImplementations...
                                   >
      , command::chunk::Description< chunk::access::Mutable
                                   , StorageImplementations...
                                   >
      , command::file::Read
      , command::file::Write
      , command::segment::Create
      , command::segment::Remove
      , command::storage::Create<StorageImplementations...>
      , command::storage::Remove
      , command::storage::Size
      , command::storage::size::Max
      , command::storage::size::Used
      >
    ;
}
