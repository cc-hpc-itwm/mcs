// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/serialization/error/NotEnoughBytes.hpp>

namespace mcs::serialization::error
{
  NotEnoughBytes::NotEnoughBytes
    ( std::size_t wanted
    , std::size_t provided
    ) noexcept
      : mcs::Error
         { fmt::format ("serialization::error::NotEnoughBytes:"
                        " {} bytes wanted but archive has only {} left."
                       , wanted
                       , provided
                       )
         }
      , _wanted {wanted}
      , _provided {provided}
  {}
  NotEnoughBytes::~NotEnoughBytes() = default;
}
