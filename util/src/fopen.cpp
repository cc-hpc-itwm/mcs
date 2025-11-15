// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fmt/format.h>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <mcs/util/fopen.hpp>
#include <stdexcept>

namespace mcs::util
{
  auto fopen (std::filesystem::path path, char const* mode) -> OpenFile
  {
    auto* file {std::fopen (path.c_str(), mode)};

    if (file == nullptr)
    {
      auto const error_code {errno};

      // \todo specific exception
      throw std::runtime_error
        { fmt::format ( "Could not open '{}'/{}: {}"
                      , path
                      , mode
                      , strerror (error_code)
                      )
        };
    }

    return OpenFile {file, FileClose {path}};
  }
}
