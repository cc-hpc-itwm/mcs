// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Interface.hpp>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  struct Client : public Interface
  {
    Client (libfabric::Interface, libfabric::Name);

    using Interface::name;
    using Interface::recv;

    auto send (std::span<std::byte const> data) -> void;

  private:
    fi_addr_t _provider;
  };
}
