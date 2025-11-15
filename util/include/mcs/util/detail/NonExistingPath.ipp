// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <utility>

namespace mcs::util
{
  template<typename... Args>
    requires (std::is_constructible_v<std::filesystem::path, Args...>)
    NonExistingPath::NonExistingPath (Args&&... args)
      : _path {std::forward<Args> (args)...}
  {
    if (std::filesystem::exists (_path))
    {
      throw Error::PathAlreadyExists {_path};
    }
  }
}
