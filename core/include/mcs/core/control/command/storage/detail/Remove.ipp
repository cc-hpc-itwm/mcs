// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::storage::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::storage::Remove>::format
      ( mcs::core::control::command::storage::Remove const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::segment::storage::Remove "
      , std::make_tuple (value.storage_id)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::storage::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::storage::Remove
  {
    prefix (state, "control::provider::segment::storage::Remove ");

    return std::make_from_tuple<mcs::core::control::command::storage::Remove>
      (parse<std::tuple<decltype (mcs::core::control::command::storage::Remove::storage_id)>> (state));
  }
}
