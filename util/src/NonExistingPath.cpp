// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/NonExistingPath.hpp>
#include <memory>

namespace mcs::util
{
  NonExistingPath::Error::PathAlreadyExists::PathAlreadyExists
    ( std::filesystem::path path
    )
      : mcs::Error
        { fmt::format ("NonExistingPath: '{}' already exists.", path)
        }
      , _path {path}
  {}
  auto NonExistingPath::Error::PathAlreadyExists::path
    (
    ) const -> std::filesystem::path
  {
    return _path;
  }
  NonExistingPath::Error::PathAlreadyExists::~PathAlreadyExists() = default;

  NonExistingPath::operator std::filesystem::path const&() const noexcept
  {
    return _path;
  }
  auto NonExistingPath::operator*() noexcept -> std::filesystem::path&
  {
    return _path;
  }
  auto NonExistingPath::operator*
    (
    ) const noexcept -> std::filesystem::path const&
  {
    return _path;
  }
  auto NonExistingPath::operator->() noexcept -> std::filesystem::path*
  {
    return std::addressof (_path);
  }
  auto NonExistingPath::operator->
    (
    ) const noexcept -> std::filesystem::path const*
  {
    return std::addressof (_path);
  }
}
