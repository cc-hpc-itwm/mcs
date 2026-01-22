// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/STD/filesystem/path.hpp>
#include <tuple>

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    template<typename ParseContext>
      constexpr auto formatter
        <mcs::core::storage::trace::event::file::Read<Storage>>
          ::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    template<typename FormatContext>
      constexpr auto formatter
        <mcs::core::storage::trace::event::file::Read<Storage>>
          ::format
            ( mcs::core::storage::trace::event::file::Read<Storage> const& file_read
            , FormatContext& context
            ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::file::Read {}"
      , std::make_tuple
        ( file_read._parameter_file_read
        , file_read._segment_id
        , file_read._offset
        , file_read._path
        , file_read._range
        )
      );
  }
}
