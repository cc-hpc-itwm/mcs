// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/serialization/define.hpp>
#include <utility>

namespace mcs::serialization
{
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
    (oa, path, std::filesystem::path)
  {
    save (oa, path.native());

    return oa;
  }
  MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
    (ia, std::filesystem::path)
  {
    auto native {load<std::filesystem::path::string_type> (ia)};

    return std::filesystem::path {native};
  }
}
