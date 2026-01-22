// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::trace::event::size::used::Result>::parse
      (ParseContext& context)
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::trace::event::size::used::Result>::format
      ( mcs::core::storage::trace::event::size::used::Result const& size_used_result
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::size::used::Result = {}"
      , size_used_result._size_used
      );
  }
}
