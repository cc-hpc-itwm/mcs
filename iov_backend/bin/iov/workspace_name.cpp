// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "workspace_name.hpp"

namespace mcs::iov_backend
{
  auto workspace_name() -> iov::meta::Name const&
  {
    static const auto _workspace_name {iov::meta::Name {"mcs_iov_backend"}};

    return _workspace_name;
  }
}
