// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <tuple>

namespace fmt
{
  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    template<typename ParseContext>
      constexpr auto formatter
        <mcs::core::storage::trace::event::chunk::Description<Storage, Access>>
          ::parse (ParseContext& context)
  {
    return context.begin();
  }

  template< mcs::core::storage::is_implementation Storage
          , mcs::core::chunk::is_access Access
          >
    template<typename FormatContext>
      constexpr auto formatter
        <mcs::core::storage::trace::event::chunk::Description<Storage, Access>>
          ::format
            ( mcs::core::storage::trace::event::chunk::Description<Storage, Access> const& chunk_description
            , FormatContext& context
            ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( chunk_description._parameter_chunk_description
        , chunk_description._segment_id
        , chunk_description._memory_range
        )
      );
  }
}
