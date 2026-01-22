// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::share_service::Chunk>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::share_service::Chunk>::format
      ( mcs::share_service::Chunk const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "share_service::Chunk "
      , std::make_tuple 
          ( value.provider_connectable
          , value.storage_id
          , value.storage_implementation_id
          , value.segment_id
          , value.size
          )
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::share_service::Chunk>::read
      ( State<Char>& state
      ) -> mcs::share_service::Chunk
  {
    prefix (state, "share_service::Chunk ");

    using Chunk = mcs::share_service::Chunk;
    return std::make_from_tuple<Chunk>
      ( parse< std::tuple
               < decltype (Chunk::provider_connectable)
               , decltype (Chunk::storage_id)
               , decltype (Chunk::storage_implementation_id)
               , decltype (Chunk::segment_id)
               , decltype (Chunk::size)
               >
             > (state)
      );
  }
}
