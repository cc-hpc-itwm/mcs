// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <type_traits>
#include <utility>

namespace mcs::util::FMT::STD::FILESYSTEM
{
  Path::Path (std::filesystem::path path) noexcept
    : _value {std::move (path)}
  {
    static_assert (std::is_nothrow_move_constructible_v<std::filesystem::path>);
  }
}
