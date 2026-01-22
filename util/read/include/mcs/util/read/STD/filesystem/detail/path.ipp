// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/parse.hpp>
#include <mcs/util/string.hpp>

namespace mcs::util::read
{
  template<typename Char>
    auto Read<std::filesystem::path>::read
      ( State<Char>& state
      ) -> std::filesystem::path
  {
    return std::filesystem::path {parse<util::string> (state)};
  }
}
