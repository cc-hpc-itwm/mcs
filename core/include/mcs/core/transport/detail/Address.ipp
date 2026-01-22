// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::transport::Address>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::transport::Address>::format
      ( mcs::core::transport::Address const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::core::transport::Address "
      , std::make_tuple 
          ( value.storage_id
          , value.storage_parameter_chunk_description
          , value.segment_id
          , value.offset
          )
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::transport::Address>::read
      ( State<Char>& state
      ) -> mcs::core::transport::Address
  {
    prefix (state, "mcs::core::transport::Address ");

    using Address = mcs::core::transport::Address;
    return std::make_from_tuple<Address>
      ( parse< std::tuple
               < decltype (Address::storage_id)
               , decltype (Address::storage_parameter_chunk_description)
               , decltype (Address::segment_id)
               , decltype (Address::offset)
               >
             > (state)
      );
  }
}
