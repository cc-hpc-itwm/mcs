// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>

namespace mcs::util::read
{
  template<typename Char>
    auto Read<Comma>::read
      ( State<Char>& state
      ) -> Comma
  {
    skip_whitespace (state);
    prefix (state, ",");
    return Comma{};
  }
}
