// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Provider.hpp>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  auto Provider::send
    ( std::span<std::byte const> data
    , libfabric::Name name
    ) -> void
  {
    return Interface::send (data, find_or_insert_name (name.value()));
  }

  auto Provider::find_or_insert_name
    ( std::vector<std::byte> name
    ) -> fi_addr_t
  {
    auto const lock {std::scoped_lock {_addresses_mutex}};

    {
      auto const address {_addresses.find (name)};

      if (address != std::end (_addresses))
      {
        return address->second;
      }
    }

    auto fi_addr {fi_addr_t {FI_ADDR_UNSPEC}};

    _av->insert_single_address
      ( name.data()
      , std::addressof (fi_addr)
      , 0
      , nullptr
      );

    auto const [address, inserted] {_addresses.emplace (name, fi_addr)};

    if (!inserted)
    {
      throw mcs::Error {"INCONSISTENCY: Duplicate address!?"};
    }

    return address->second;
  }
}
