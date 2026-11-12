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
        ( file_read.parameter_file_read
        , file_read.segment_id
        , file_read.offset
        , mcs::util::FMT::STD::FILESYSTEM::Path {file_read.path}
        , file_read.range
        )
      );
  }
}
