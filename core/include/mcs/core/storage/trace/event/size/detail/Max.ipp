// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <tuple>

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::trace::event::size::Max<Storage>>::parse
        (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::trace::event::size::Max<Storage>>::format
        ( mcs::core::storage::trace::event::size::Max<Storage> const& size_max
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::size::Max {}"
      , std::make_tuple
        ( size_max._parameter_size_max
        )
      );
  }
}
