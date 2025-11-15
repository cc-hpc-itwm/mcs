// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ostream.h>
#include <utility>

namespace mcs::util::FMT
{
  template<typename... Args>
    auto write_file ( std::filesystem::path path
                    , fmt::format_string<Args...> format
                    , Args&&... args
                    ) -> std::filesystem::path
  try
  {
    auto stream {std::ofstream {path, std::ios::out | std::ios::trunc}};
    stream.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fmt::print (stream, format, std::forward<Args> (args)...);
    return path;
  }
  catch (...)
  {
    std::throw_with_nested (WriteFileFailed {path});
  }
}
