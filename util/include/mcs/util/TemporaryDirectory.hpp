// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/util/NonExistingPath.hpp>
#include <memory>

namespace mcs::util
{
  // On destruction: Removes the path and everything below.
  //
  struct TemporaryDirectory
  {
    // Creates the last level of the directory.
    //
    // Note: RACE: Concurrent constructions might all determine the
    // path does not exist before any of them creates the directory
    // and they all will be happy as creating an existing directory is
    // not an error.
    //
    // Ensures: path does exist
    //
    TemporaryDirectory (NonExistingPath);

    auto path() const noexcept -> std::filesystem::path const&;

  private:
    struct Deleter
    {
      auto operator() (std::filesystem::path*) const noexcept -> void;
    };
    std::unique_ptr<std::filesystem::path, Deleter> _path;
  };
}
