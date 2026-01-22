// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load_from.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/STD/vector.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>
#include <tuple>
#include <utility>

namespace mcs::core::storage
{
  template<serialization::is_serializable T>
    Parameter::Parameter (Make, T const& x)
      : Parameter {serialization::OArchive {x}.bytes()}
  {}

  template<serialization::is_serializable T>
    auto Parameter::as() const -> T
  {
    return serialization::load_from<T> (_blob);
  }

  template<serialization::is_serializable T>
    auto make_parameter (T const& x) -> Parameter
  {
    return Parameter {Parameter::Make{}, x};
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::Parameter>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::Parameter>::format
      ( mcs::core::storage::Parameter const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}{}", "Parameter ", value._blob);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::Parameter>::read
      ( State<Char>& state
      ) -> mcs::core::storage::Parameter
  {
    prefix (state, "Parameter ");

    return mcs::core::storage::Parameter
      { parse<decltype (mcs::core::storage::Parameter::_blob)> (state)
      };
  }
}
