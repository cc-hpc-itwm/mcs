// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/transport/implementation/libfabric/command/Get.hpp>
#include <mcs/core/transport/implementation/libfabric/command/Put.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::core::transport::implementation::libfabric
{
  using Commands = util::type::List
    < command::Get
    , command::Put
    >;
}
