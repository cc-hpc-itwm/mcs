// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/core/transport/implementation/libfabric/libfabric/Domain.hpp>
#include <string>
#include <utility>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  Domain::Domain (std::string value)
    : _value {std::move (value)}
  {}
  auto Domain::value() const -> std::string
  {
    return _value;
  }
}
