// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::block_device::Storage>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::block_device::Storage>::format
      ( mcs::block_device::Storage const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::block_device::Storage "
      , std::make_tuple 
          ( value._provider_connectable
          , value._storage_id
          , value._storage_parameter_chunk_description
          , value._segment_id
          , value._range
          )
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::block_device::Storage>::read
      ( State<Char>& state
      ) -> mcs::block_device::Storage
  {
    prefix (state, "mcs::block_device::Storage ");

    using Storage = mcs::block_device::Storage;
    return std::make_from_tuple<Storage>
      ( parse< std::tuple
               < decltype (Storage::_provider_connectable)
               , decltype (Storage::_storage_id)
               , decltype (Storage::_storage_parameter_chunk_description)
               , decltype (Storage::_segment_id)
               , decltype (Storage::_range)
               >
             > (state)
      );
  }
}
