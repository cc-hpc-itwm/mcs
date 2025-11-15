// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/error/Load.hpp>
#include <mcs/util/FMT/STD/exception.hpp>

namespace mcs::serialization::error
{
  Load::Load() noexcept
    : mcs::Error {"serialization::error::Load"}
  {}
  Load::~Load() = default;
}
