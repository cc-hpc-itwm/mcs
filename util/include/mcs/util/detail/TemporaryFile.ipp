// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/write_file.hpp>

namespace mcs::util
{
  template<typename... Args>
    TemporaryFile::TemporaryFile
      ( NonExistingPath path
      , fmt::format_string<Args...> format
      , Args&&... args
      )
        : _path { new std::filesystem::path {std::filesystem::absolute (path)}
                , {}
                }
  {
    util::FMT::write_file (*_path, format, std::forward<Args> (args)...);
  }
}
