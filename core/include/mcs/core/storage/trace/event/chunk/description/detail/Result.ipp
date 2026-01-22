// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    template<typename ParseContext>
      constexpr auto formatter
        < mcs::core::storage::trace::event::chunk::description
            ::Result<Storage, Access>
        >::parse (ParseContext& context)
  {
    return context.begin();
  }

  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    template<typename FormatContext>
      constexpr auto formatter
        < mcs::core::storage::trace::event::chunk::description
            ::Result<Storage, Access>
        >::format
          ( mcs::core::storage::trace::event::chunk::description
              ::Result<Storage, Access> const& chunk_description_result
          , FormatContext& context
          ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::chunk::description::Result<{}> = {}"
      , Access{}
      , chunk_description_result._chunk_description
      );
  }
}
