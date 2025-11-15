// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/TemporaryFile.hpp>

namespace mcs::util
{
  auto TemporaryFile::Deleter::operator()
    ( std::filesystem::path* path
    ) const noexcept -> void
  {
    auto ec {std::error_code{}};
    std::filesystem::remove (*path, ec);
    std::ignore = ec;
    std::default_delete<std::filesystem::path>{} (path);
  }

  auto TemporaryFile::path() const noexcept -> std::filesystem::path const&
  {
    return *_path;
  }
}
