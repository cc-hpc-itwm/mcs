// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/iov_backend/Storage.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <unordered_map>

namespace mcs::iov_backend::provider::state
{
  // \todo hide STL
  using Storages = std::unordered_map<storage::ID, Storage>;
}
