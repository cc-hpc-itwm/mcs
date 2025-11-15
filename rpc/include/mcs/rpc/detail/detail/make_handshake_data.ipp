// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <string>
#include <typeinfo>
#include <vector>

namespace mcs::rpc::detail
{
  template<is_command... Commands>
    MCS_CXX23_CONSTEXPR auto make_handshake_data()
  {
    auto strings {std::vector<std::string>{}};
    (strings.emplace_back (typeid (Commands).name()), ...);

    return strings;
  }
}
