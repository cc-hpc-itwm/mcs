// Copyright (C) 2024-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::file::Read>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::file::Read>::format
      ( mcs::core::control::command::file::Read const& value
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{}{}"
      , "control::provider::file::Read "
      , std::make_tuple 
          ( value.storage_id
          , value.parameter_file_read
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
    auto Read<mcs::core::control::command::file::Read>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::file::Read
  {
    prefix (state, "control::provider::file::Read ");

    using Read = mcs::core::control::command::file::Read;
    return std::make_from_tuple<Read>
      ( parse< std::tuple
               < decltype (Read::storage_id)
               , decltype (Read::parameter_file_read)
               , decltype (Read::segment_id)
               , decltype (Read::offset)
               , decltype (Read::file)
               , decltype (Read::range)
               >
             > (state)
      );
  }
}
