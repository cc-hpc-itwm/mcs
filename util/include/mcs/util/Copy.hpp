// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <filesystem>
#include <mcs/util/OpenFile.hpp>
#include <sys/types.h>

namespace mcs::util::copy
{
  struct FileLocation
  {
    FileLocation (std::filesystem::path, char const* mode, off_t);

    util::OpenFile _open_file;
    int _fd;
  };

  struct FileReadLocation : public FileLocation
  {
    FileReadLocation (std::filesystem::path, off_t);
  };

  struct FileWriteLocation : public FileLocation
  {
    FileWriteLocation (std::filesystem::path, off_t);
  };
}

namespace mcs::util
{
  struct Copy
  {
    auto operator()
      ( copy::FileReadLocation const&
      , copy::FileWriteLocation const&
      , std::size_t
      ) const -> std::size_t
      ;

    auto operator()
      ( std::byte const*
      , copy::FileWriteLocation const&
      , std::size_t
      ) const -> std::size_t
      ;

    auto operator()
      ( copy::FileReadLocation const&
      , std::byte*
      , std::size_t
      ) const -> std::size_t
      ;
  };
}
