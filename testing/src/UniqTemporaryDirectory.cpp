// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/UniqTemporaryDirectory.hpp>

namespace mcs::testing
{
  auto UniqTemporaryDirectory::path() const -> std::filesystem::path
  {
    return _temporary_data_path.path();
  }
}
