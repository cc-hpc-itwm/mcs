// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <utility>

namespace mcs::serialization
{
  auto Implementation<std::filesystem::path>::output
    ( OArchive& oa
    , std::filesystem::path const& path
    ) -> OArchive&
  {
    save (oa, path.native());

    return oa;
  }
  auto Implementation<std::filesystem::path>::input
    ( IArchive& ia
    ) -> std::filesystem::path
  {
    auto native {load<std::filesystem::path::string_type> (ia)};

    return std::filesystem::path {native};
  }
}
