// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/string.hpp>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::util::FMT::STD::FILESYSTEM::Path>::parse
      ( ParseContext& context
      )
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::util::FMT::STD::FILESYSTEM::Path>::format
      ( mcs::util::FMT::STD::FILESYSTEM::Path const& path
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{}"
      , mcs::util::string {path._value.string()}
      );
  }
}
