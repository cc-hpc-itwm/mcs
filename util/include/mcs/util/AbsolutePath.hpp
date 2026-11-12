// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <filesystem>
#include <mcs/Error.hpp>

namespace mcs::util
{
  struct AbsolutePath
  {
    struct Error
    {
      struct MustBeAbsolute : public mcs::Error
      {
        explicit MustBeAbsolute (std::filesystem::path);
        ~MustBeAbsolute() override = default;
        MustBeAbsolute (MustBeAbsolute const&) = default;
        MustBeAbsolute (MustBeAbsolute&&) noexcept = default;
        auto operator= (MustBeAbsolute const&) -> MustBeAbsolute& = default;
        auto operator= (MustBeAbsolute&&) noexcept -> MustBeAbsolute& = default;
      };
    };

    [[nodiscard]] AbsolutePath (std::filesystem::path);

    [[nodiscard]] auto path() const -> std::filesystem::path const&;

    [[nodiscard]] auto operator<=> (AbsolutePath const&) const = default;

  private:
    std::filesystem::path _path;
  };
}
