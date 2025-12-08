// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/libfabric/Provider.hpp>
#include <string>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  Provider::Provider (std::string value)
    : _value {value}
  {}
  auto Provider::value() const -> std::string
  {
    return _value;
  }
}
