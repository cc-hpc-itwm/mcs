// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/chunk/access/Const.hpp>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <mcs/share_service/command/Attach.hpp>
#include <mcs/share_service/command/Create.hpp>
#include <mcs/share_service/command/Remove.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::share_service
{
  using Commands = util::type::List
    < command::Attach<core::chunk::access::Const>
    , command::Attach<core::chunk::access::Mutable>
    , command::Create
    , command::Remove
    >;
}
