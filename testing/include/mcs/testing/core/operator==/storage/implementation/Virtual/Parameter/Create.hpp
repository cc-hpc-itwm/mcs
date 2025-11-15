// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Virtual.hpp>

namespace mcs::core::storage::implementation
{
  [[nodiscard]] auto operator==
    ( Virtual::Parameter::Create const&
    , Virtual::Parameter::Create const&
    ) -> bool
    ;
}
