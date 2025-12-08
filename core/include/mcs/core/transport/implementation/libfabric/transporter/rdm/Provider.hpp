// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <algorithm>
#include <map>
#include <mcs/core/transport/implementation/libfabric/transporter/rdm/Interface.hpp>
#include <mutex>
#include <vector>

namespace mcs::core::transport::implementation::libfabric::transporter::rdm
{
  struct Provider : public Interface
  {
    using Interface::Interface;
    using Interface::name;
    using Interface::recv;

    auto send (std::span<std::byte const>, libfabric::Name) -> void;

  private:
    auto find_or_insert_name (std::vector<std::byte> name) -> fi_addr_t;
    std::mutex _addresses_mutex;

#if defined (MCS_CONFIG_GCC_WORKAROUND_FALSE_WARNING_WITH_DEFAULT_COMPARE)
    struct ByteVectorComparator
    {
      auto operator()
        ( const std::vector<std::byte>& lhs
        , const std::vector<std::byte>& rhs
        ) const -> bool
      {
#if defined (MCS_CONFIG_GCC_WORKAROUND_FALSE_WARNING_WITH_RANGES_COMPARE)
        return std::lexicographical_compare
          ( lhs.begin()
          , lhs.end()
          , rhs.begin()
          , rhs.end()
          );
#else
        return std::ranges::lexicographical_compare (lhs, rhs);
#endif
      }
    };
    std::map<std::vector<std::byte>, fi_addr_t, ByteVectorComparator> _addresses;
#else
    std::map<std::vector<std::byte>, fi_addr_t> _addresses;
#endif
  };
}
