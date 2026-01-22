// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::trace::event::Destruct>
      ::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::trace::event::Destruct>
      ::format
        ( mcs::core::storage::trace::event::Destruct const& /* destruct */
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "trace::event::Destruct");
  }
}
