// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::storage::size::Used>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::storage::size::Used>::format
      ( mcs::core::control::command::storage::size::Used const& value
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "{}{}"
      , "control::provider::segment::storage::size::Used "
      , std::make_tuple (value.storage_id, value.storage_parameter)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::storage::size::Used>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::storage::size::Used
  {
    prefix (state, "control::provider::segment::storage::size::Used ");

    using Used = mcs::core::control::command::storage::size::Used;
    return std::make_from_tuple<Used>
      ( parse< std::tuple
               < decltype (Used::storage_id)
               , decltype (Used::storage_parameter)
               >
             > (state)
      );
  }
}
