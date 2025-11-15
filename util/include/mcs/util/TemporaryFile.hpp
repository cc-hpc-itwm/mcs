// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <mcs/util/NonExistingPath.hpp>
#include <memory>

namespace mcs::util
{
  // On destruction: Removes the file.
  //
  struct TemporaryFile
  {
    // Creates a temporary file with content.
    //
    // Expects: path does not exists
    // Ensures: path does exist with the given content
    //
    // Note: RACE: Concurrent constructions might all determine the
    // file does not exist before any of them creates the file and
    // they will overwrite each others content.
    //
    // EXAMPLE:
    //
    //     auto file {TemporaryFile {path, "{}", value}};
    //
    template<typename... Args>
      TemporaryFile ( NonExistingPath
                    , fmt::format_string<Args...>
                    , Args&&...
                    );

    auto path() const noexcept -> std::filesystem::path const&;

  private:
    struct Deleter
    {
      auto operator() (std::filesystem::path*) const noexcept -> void;
    };
    std::unique_ptr<std::filesystem::path, Deleter> _path;
  };
}

#include "detail/TemporaryFile.ipp"

// \todo test
