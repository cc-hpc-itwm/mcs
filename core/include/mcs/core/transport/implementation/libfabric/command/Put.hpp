// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Size.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::core::transport::implementation::libfabric::command
{
  struct Put
  {
    using Response = memory::Size;

    transport::Address destination;
    memory::Size size;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::transport::implementation::libfabric::command::Put>
  {
    using Type = core::transport::implementation::libfabric::command::Put;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}
