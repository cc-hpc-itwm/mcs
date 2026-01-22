// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::trace::Tag<Tracer, Storage>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<typename Tracer, mcs::core::storage::is_implementation Storage>
    requires (mcs::core::storage::trace::is_tracer<Tracer, Storage>)
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::trace::Tag<Tracer, Storage>>::format
        ( mcs::core::storage::implementation::trace::Tag<Tracer, Storage> const& /* tag */
        , FormatContext& context
        ) const -> decltype (context.out())
  {
    return fmt::format
      ( context.out()
      , "Traced<{},{}>"
      , typename Tracer::Tag{}
      , typename Storage::Tag{}
      );
  }
}
