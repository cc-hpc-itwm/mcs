// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::storage::size::Max>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::storage::size::Max>::format
      ( mcs::core::control::command::storage::size::Max const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::segment::storage::size::Max "
      , std::make_tuple (value.storage_id, value.storage_parameter)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::storage::size::Max>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::storage::size::Max
  {
    prefix (state, "control::provider::segment::storage::size::Max ");

    using Max = mcs::core::control::command::storage::size::Max;
    return std::make_from_tuple<Max>
      ( parse< std::tuple
               < decltype (Max::storage_id)
               , decltype (Max::storage_parameter)
               >
             > (state)
      );
  }
}
