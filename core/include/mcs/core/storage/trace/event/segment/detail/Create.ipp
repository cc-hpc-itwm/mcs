// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <tuple>

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::trace::event::segment::Create<Storage>>::parse
        (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::trace::event::segment::Create<Storage>>::format
        ( mcs::core::storage::trace::event::segment::Create<Storage> const& segment_create
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::segment::Create {}"
      , std::make_tuple
        ( segment_create._parameter_segment_create
        , segment_create._size
        )
      );
  }
}
