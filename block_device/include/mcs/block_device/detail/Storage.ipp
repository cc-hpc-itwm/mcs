// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::block_device::Storage>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::block_device::Storage>::format
      ( mcs::block_device::Storage const& value
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{}{}"
      , "mcs::block_device::Storage "
      , std::make_tuple
          ( value.provider_connectable
          , value.storage_id
          , value.storage_parameter_chunk_description
          , value.segment_id
          , value.range
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
               < decltype (Storage::provider_connectable)
               , decltype (Storage::storage_id)
               , decltype (Storage::storage_parameter_chunk_description)
               , decltype (Storage::segment_id)
               , decltype (Storage::range)
               >
             > (state)
      );
  }
}
