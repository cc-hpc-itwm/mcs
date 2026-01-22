// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace mcs::core::control::command::storage
{
  constexpr Size::Response::Response
    ( core::storage::MaxSize max
    , core::memory::Size used
    ) noexcept
      : _max {max}
      , _used {used}
  {}
  constexpr auto Size::Response::max
    (
    ) const noexcept -> core::storage::MaxSize
  {
    return _max;
  }
  constexpr auto Size::Response::used
    (
    ) const noexcept -> core::memory::Size
  {
    return _used;
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::storage::Size::Response>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::storage::Size::Response>::format
      ( mcs::core::control::command::storage::Size::Response const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::segment::storage::Size::Response "
      , std::make_tuple (value._max, value._used)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::storage::Size::Response>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::storage::Size::Response
  {
    prefix (state, "control::provider::segment::storage::Size::Response ");

    using Response = mcs::core::control::command::storage::Size::Response;
    return std::make_from_tuple<Response>
      ( parse< std::tuple
               < decltype (Response::_max)
               , decltype (Response::_used)
               >
             > (state)
      );
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::control::command::storage::Size>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::control::command::storage::Size>::format
      ( mcs::core::control::command::storage::Size const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "control::provider::segment::storage::Size "
      , std::make_tuple 
          ( value.storage_id
          , value.storage_parameter_size_max
          , value.storage_parameter_size_used
          )
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::control::command::storage::Size>::read
      ( State<Char>& state
      ) -> mcs::core::control::command::storage::Size
  {
    prefix (state, "control::provider::segment::storage::Size ");

    using Size = mcs::core::control::command::storage::Size;
    return std::make_from_tuple<Size>
      ( parse< std::tuple
               < decltype (Size::storage_id)
               , decltype (Size::storage_parameter_size_max)
               , decltype (Size::storage_parameter_size_used)
               >
             > (state)
      );
  }
}
