// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::testing::random
{
  template<typename... Args>
    value<mcs::util::string>::value (Args&&... args)
      : _random_string {std::forward<Args> (args)...}
  {}
}
