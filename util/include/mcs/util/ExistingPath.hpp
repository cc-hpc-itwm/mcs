// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <mcs/Error.hpp>
#include <type_traits>

namespace mcs::util
{
  // Non-explicit wrapper around std::filesystem::path that throws
  // when constructed with an non-existing path.
  //
  struct ExistingPath
  {
    template<typename... Args>
      requires (std::is_constructible_v<std::filesystem::path, Args...>)
      ExistingPath (Args&&...);

    struct Error
    {
      struct PathDoesNotExist : public mcs::Error
      {
        auto path() const -> std::filesystem::path;

        MCS_ERROR_COPY_MOVE_DEFAULT (PathDoesNotExist);

      private:
        friend struct ExistingPath;

        PathDoesNotExist (std::filesystem::path);

        std::filesystem::path _path;
      };
    };

    operator std::filesystem::path const&() const noexcept;

    auto operator*() noexcept -> std::filesystem::path&;
    auto operator*() const noexcept -> std::filesystem::path const&;
    auto operator->() noexcept -> std::filesystem::path*;
    auto operator->() const noexcept -> std::filesystem::path const*;

  private:
    std::filesystem::path _path;
  };
}

#include "detail/ExistingPath.ipp"
