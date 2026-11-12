// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/libfabric/Provider.hpp>
#include <string>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  Provider::Provider (std::string value)
    : _value {std::move (value)}
  {}
  auto Provider::value() const -> std::string
  {
    return _value;
  }
}
