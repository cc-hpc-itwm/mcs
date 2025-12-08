// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <string>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  struct Domain
  {
    explicit Domain (std::string);

    [[nodiscard]] auto value() const -> std::string;

  private:
    std::string _value;
  };
}
