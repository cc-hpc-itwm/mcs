// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::segment::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::segment::Remove>::format
      ( mcs::core::control::command::segment::Remove const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::segment::Remove "
      , std::make_tuple (value.storage_id, value.storage_parameter, value.segment_id)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::segment::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::segment::Remove
  {
    prefix (state, "control::provider::segment::Remove ");

    using Remove = mcs::core::control::command::segment::Remove;
    return std::make_from_tuple<Remove>
      ( parse< std::tuple
               < decltype (Remove::storage_id)
               , decltype (Remove::storage_parameter)
               , decltype (Remove::segment_id)
               >
             > (state)
      );
  }
}
