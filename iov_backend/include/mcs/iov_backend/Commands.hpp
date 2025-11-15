// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/command/Locations.hpp>
#include <mcs/iov_backend/command/Range.hpp>
#include <mcs/iov_backend/command/State.hpp>
#include <mcs/iov_backend/command/collection/Append.hpp>
#include <mcs/iov_backend/command/collection/Create.hpp>
#include <mcs/iov_backend/command/collection/Delete.hpp>
#include <mcs/iov_backend/command/storage/Add.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::iov_backend
{
  using Commands = util::type::List
    < command::Range
    , command::Locations
    , command::State
    , command::collection::Append
    , command::collection::Create
    , command::collection::Delete
    , command::storage::Add
    >;
}
