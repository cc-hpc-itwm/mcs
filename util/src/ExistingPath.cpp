// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/ExistingPath.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <memory>

namespace mcs::util
{
  ExistingPath::Error::PathDoesNotExist::PathDoesNotExist
    ( std::filesystem::path path
    )
      : mcs::Error
        { fmt::format ("ExistingPath: '{}' does not exist.", path)
        }
      , _path {path}
  {}
  auto ExistingPath::Error::PathDoesNotExist::path
    (
    ) const -> std::filesystem::path
  {
    return _path;
  }
  ExistingPath::Error::PathDoesNotExist::~PathDoesNotExist() = default;

  ExistingPath::operator std::filesystem::path const&() const noexcept
  {
    return _path;
  }
  auto ExistingPath::operator*() noexcept -> std::filesystem::path&
  {
    return _path;
  }
  auto ExistingPath::operator*() const noexcept -> std::filesystem::path const&
  {
    return _path;
  }
  auto ExistingPath::operator->() noexcept -> std::filesystem::path*
  {
    return std::addressof (_path);
  }
  auto ExistingPath::operator->() const noexcept -> std::filesystem::path const*
  {
    return std::addressof (_path);
  }
}
