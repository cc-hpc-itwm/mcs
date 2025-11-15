// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <fmt/format.h>
#include <fstream>
#include <iterator>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <stdexcept>
#include <string>

namespace mcs::util
{
  auto read_file (std::filesystem::path const& file) -> std::string
  {
    if (!std::filesystem::exists (file))
    {
      // \todo specific exception
      throw std::invalid_argument
        {fmt::format ("read::State: '{}' does not exists", file)};
    }

    auto ifs {std::ifstream {file}};

    if (!ifs)
    {
      // \todo specific exception
      throw std::runtime_error
        {fmt::format ("could not open '{}': ", file, strerror (errno))};
    }

    ifs >> std::noskipws;

    return { std::istream_iterator<char> (ifs)
           , std::istream_iterator<char>{}
           };
  }
}
