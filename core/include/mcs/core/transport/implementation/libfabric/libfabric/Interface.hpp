// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/transport/implementation/libfabric/libfabric/Domain.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Provider.hpp>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  struct Interface
  {
    Provider provider;
    Domain domain;
  };
}
