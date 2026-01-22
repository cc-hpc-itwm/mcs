// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::file::Read>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::file::Read>::format
      ( mcs::core::control::command::file::Read const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::file::Read "
      , std::make_tuple 
          ( value._storage_id
          , value._parameter_file_read
          , value._segment_id
          , value._offset
          , value._file
          , value._range
          )
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::file::Read>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::file::Read
  {
    prefix (state, "control::provider::file::Read ");

    using Read = mcs::core::control::command::file::Read;
    return std::make_from_tuple<Read>
      ( parse< std::tuple
               < decltype (Read::_storage_id)
               , decltype (Read::_parameter_file_read)
               , decltype (Read::_segment_id)
               , decltype (Read::_offset)
               , decltype (Read::_file)
               , decltype (Read::_range)
               >
             > (state)
      );
  }
}
