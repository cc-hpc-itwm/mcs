// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::trace::event::size::max::Result>::parse
      (ParseContext& context)
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::trace::event::size::max::Result>::format
      ( mcs::core::storage::trace::event::size::max::Result const& size_max_result
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::size::max::Result = {}"
      , size_max_result._max_size
      );
  }
}
