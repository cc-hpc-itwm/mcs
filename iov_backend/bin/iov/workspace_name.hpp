// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <iov/meta.hpp>

namespace mcs::iov_backend
{
  auto workspace_name() -> iov::meta::Name const&;
}
