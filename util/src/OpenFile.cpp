// Copyright (C) 2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/OpenFile.hpp>
#include <utility>

namespace mcs::util
{
  FileClose::FileClose (std::filesystem::path path)
    : _path {std::move (path)}
  {}

  auto FileClose::operator() (std::FILE* file) const noexcept -> void
  {
    if (std::fclose (file) != 0)
    {
      auto const error_code {errno};

      util::FMT::print_noexcept
        ( stderr
        , "Error when closing '{}': {}."
        , util::FMT::STD::FILESYSTEM::Path {_path}
        , strerror (error_code)
        );
    }
  }
}
