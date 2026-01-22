// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/string.hpp>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<std::filesystem::path>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<std::filesystem::path>::format
      ( std::filesystem::path const& path
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}", mcs::util::string {path.string()});
  }
}
