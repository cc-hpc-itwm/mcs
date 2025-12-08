// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <vector>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  struct Name
  {
    [[nodiscard]] Name (std::vector<std::byte>) noexcept;

    [[nodiscard]] auto value() const noexcept -> std::vector<std::byte> const&;
    [[nodiscard]] operator std::vector<std::byte> const&() const noexcept;

  private:
    MCS_SERIALIZATION_ACCESS();
    std::vector<std::byte> _value;
  };
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE
      (mcs::core::transport::implementation::libfabric::libfabric::Name)
    ;
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::transport::implementation::libfabric::libfabric::Name)
    ;
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::transport::implementation::libfabric::libfabric::Name)
    ;
}

#include "detail/Name.ipp"
