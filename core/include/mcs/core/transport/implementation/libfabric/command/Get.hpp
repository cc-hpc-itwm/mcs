// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/core/transport/implementation/libfabric/libfabric/Name.hpp>
#include <mcs/serialization/declare.hpp>

namespace mcs::core::transport::implementation::libfabric::command
{
  struct Get
  {
    using Response = memory::Size;

    transport::Address source;
    memory::Size size;
    libfabric::Name name;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::transport::implementation::libfabric::command::Get)
    ;
}
