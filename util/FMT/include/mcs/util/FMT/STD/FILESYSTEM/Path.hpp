// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>

namespace mcs::util::FMT::STD::FILESYSTEM
{
  struct Path
  {
    [[nodiscard]] Path (std::filesystem::path) noexcept;

  private:
    friend struct fmt::formatter<Path>;

    std::filesystem::path _value;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::util::FMT::STD::FILESYSTEM::Path>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::FMT::STD::FILESYSTEM::Path const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

#include "detail/Path.ipp"
