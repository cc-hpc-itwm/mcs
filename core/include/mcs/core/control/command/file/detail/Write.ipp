// Copyright (C) 2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::file::Write>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::file::Write>::format
      ( mcs::core::control::command::file::Write const& value
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{}{}"
      , "control::provider::file::Write "
      , std::make_tuple 
          ( value.storage_id
          , value.parameter_file_write
          , value.segment_id
          , value.offset
          , value.file
          , value.range
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
               < decltype (Write::storage_id)
               , decltype (Write::parameter_file_write)
               , decltype (Write::segment_id)
               , decltype (Write::offset)
               , decltype (Write::file)
               , decltype (Write::range)
               >
             > (state)
      );
  }
}
