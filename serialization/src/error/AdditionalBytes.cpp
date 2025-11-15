// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/serialization/error/AdditionalBytes.hpp>

namespace mcs::serialization::error
{
  AdditionalBytes::AdditionalBytes
    ( std::size_t additional_bytes
    ) noexcept
      : mcs::Error
         {fmt::format ("serialization::error::AdditionalBytes:"
                       " {} bytes left in archive after load has returned."
                      , additional_bytes
                      )
         }
  {}
  AdditionalBytes::~AdditionalBytes() = default;
}
