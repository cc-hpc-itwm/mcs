// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/error/WrongTag.hpp>

namespace mcs::serialization::error
{
  WrongTag::WrongTag() noexcept
    : mcs::Error {"serialization::error::WrongTag"}
  {}
  WrongTag::~WrongTag() = default;
}
