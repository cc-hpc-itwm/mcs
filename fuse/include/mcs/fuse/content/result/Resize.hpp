// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/content/result/resize/Error.hpp>
#include <mcs/fuse/content/result/resize/Success.hpp>
#include <variant>

namespace mcs::fuse::content::result
{
  using Resize = std::variant
    < resize::Success
    , resize::Error
    >;
}
