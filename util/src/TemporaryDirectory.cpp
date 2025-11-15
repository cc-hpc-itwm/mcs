// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/TemporaryDirectory.hpp>

namespace mcs::util
{
  TemporaryDirectory::TemporaryDirectory (NonExistingPath path)
    : _path {new std::filesystem::path {std::filesystem::absolute (path)}, {}}
  {
    std::filesystem::create_directory (*_path);
  }

  auto TemporaryDirectory::Deleter::operator()
    ( std::filesystem::path* path
    ) const noexcept -> void
  {
    auto ec {std::error_code{}};
    std::filesystem::remove_all (*path, ec);
    std::ignore = ec;
    std::default_delete<std::filesystem::path>{} (path);
  }

  auto TemporaryDirectory::path() const noexcept -> std::filesystem::path const&
  {
    return *_path;
  }
}
