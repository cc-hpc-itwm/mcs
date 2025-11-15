// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdio>
#include <filesystem>
#include <memory>

namespace mcs::util
{
  struct FileClose
  {
    explicit FileClose (std::filesystem::path);
    auto operator() (std::FILE*) const noexcept -> void;

  private:
    std::filesystem::path _path;
  };

  using OpenFile = std::unique_ptr<std::FILE, FileClose>;
}
