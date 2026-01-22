// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::trace::event::segment::create::Result>::parse
      (ParseContext& context)
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::trace::event::segment::create::Result>::format
      ( mcs::core::storage::trace::event::segment::create::Result const& segment_create_result
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::segment::create::Result = {}"
      , segment_create_result._segment_id
      );
  }
}
