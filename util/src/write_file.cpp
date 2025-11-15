// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/FMT/write_file.hpp>

namespace mcs::util::FMT
{
  WriteFileFailed::WriteFileFailed
    ( std::filesystem::path path
    )
      : mcs::Error {fmt::format ("write_file into '{}' failed", path)}
      , _path {path}
  {}
  WriteFileFailed::~WriteFileFailed() = default;

  auto WriteFileFailed::path() const -> std::filesystem::path
  {
    return _path;
  }
}
