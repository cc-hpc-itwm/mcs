// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/FMT/STD/FILESYSTEM/Path.hpp>
#include <tuple>

namespace fmt
{
  template<mcs::core::storage::is_implementation Storage>
    template<typename ParseContext>
      constexpr auto formatter
        <mcs::core::storage::trace::event::file::Write<Storage>>
          ::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::storage::is_implementation Storage>
    template<typename FormatContext>
      constexpr auto formatter
        <mcs::core::storage::trace::event::file::Write<Storage>>
          ::format
            ( mcs::core::storage::trace::event::file::Write<Storage> const& file_write
            , FormatContext& context
            ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "trace::event::file::Write {}"
      , std::make_tuple
        ( file_write.parameter_file_write
        , file_write.segment_id
        , file_write.offset
        , mcs::util::FMT::STD::FILESYSTEM::Path {file_write.path}
        , file_write.range
        )
      );
  }
}
