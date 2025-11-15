// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/load_from.hpp>

namespace mcs::rpc::detail
{
  template<typename T>
    auto Buffer::load() -> T
  {
    return serialization::load_from<T> (data<std::byte const>());
  }
}
