// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/AbsolutePath.hpp>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <utility>

namespace mcs::util
{
  AbsolutePath::Error::MustBeAbsolute::MustBeAbsolute
    ( std::filesystem::path path
    )
      : mcs::Error
        { fmt::format ( "{} must be absolute"
                      , FMT::STD::FILESYSTEM::Path {path}
                      )
        }
  {}

  AbsolutePath::AbsolutePath (std::filesystem::path path)
    : _path {std::move (path)}
  {
    if (!_path.is_absolute())
    {
      throw Error::MustBeAbsolute {_path};
    }
  }

  auto AbsolutePath::path() const -> std::filesystem::path const&
  {
    return _path;
  }
}
