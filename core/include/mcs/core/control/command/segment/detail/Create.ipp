// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::segment::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::segment::Create>::format
      ( mcs::core::control::command::segment::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::segment::Create "
      , std::make_tuple (value.storage_id, value.storage_parameter, value.memory_size)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::segment::Create>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::segment::Create
  {
    prefix (state, "control::provider::segment::Create ");

    using Create = mcs::core::control::command::segment::Create;
    return std::make_from_tuple<Create>
      ( parse< std::tuple
               < decltype (Create::storage_id)
               , decltype (Create::storage_parameter)
               , decltype (Create::memory_size)
               >
             > (state)
      );
  }
}
