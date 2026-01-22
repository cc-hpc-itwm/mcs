// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::file::Write>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::file::Write>::format
      ( mcs::core::control::command::file::Write const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::file::Write "
      , std::make_tuple 
          ( value._storage_id
          , value._parameter_file_write
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
    auto Read<mcs::core::control::command::file::Write>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::file::Write
  {
    prefix (state, "control::provider::file::Write ");

    using Write = mcs::core::control::command::file::Write;
    return std::make_from_tuple<Write>
      ( parse< std::tuple
               < decltype (Write::_storage_id)
               , decltype (Write::_parameter_file_write)
               , decltype (Write::_segment_id)
               , decltype (Write::_offset)
               , decltype (Write::_file)
               , decltype (Write::_range)
               >
             > (state)
      );
  }
}
