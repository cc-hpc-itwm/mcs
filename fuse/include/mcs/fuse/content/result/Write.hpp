// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/content/result/write/Error.hpp>
#include <mcs/fuse/content/result/write/Success.hpp>
#include <variant>

namespace mcs::fuse::content::result
{
  using Write = std::variant
    < write::Success
    , write::Error
    >;
}
