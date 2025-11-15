// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <mcs/Error.hpp>

namespace mcs::util::FMT
{
  // Write formatted into a file
  //
  // EXAMPLE
  //
  //     mcs::util::FMT::write_file (file_name, "{}", value);
  //
  template<typename... Args>
    auto write_file ( std::filesystem::path
                    , fmt::format_string<Args...>
                    , Args&&...
                    ) -> std::filesystem::path
    ;

  // Thrown by write_file in case of an error. Thrown as outer
  // exception of a nested exception with the reason.
  //
  struct WriteFileFailed : public mcs::Error
  {
    auto path() const -> std::filesystem::path;

    MCS_ERROR_COPY_MOVE_DEFAULT (WriteFileFailed);

  private:
    template<typename... Args> friend auto write_file
      ( std::filesystem::path
      , fmt::format_string<Args...>
      , Args&&...
      ) -> std::filesystem::path;

    WriteFileFailed (std::filesystem::path);

    std::filesystem::path _path;
  };
}

#include "detail/write_file.ipp"
