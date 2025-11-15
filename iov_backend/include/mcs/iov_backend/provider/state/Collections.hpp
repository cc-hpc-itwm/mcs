// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/iov_backend/collection/ID.hpp>
#include <unordered_map>

namespace mcs::iov_backend::provider::state
{
  // \todo hide STL
  using Collections = std::unordered_map<collection::ID, UsedStorages>;
}
