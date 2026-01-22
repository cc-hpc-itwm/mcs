// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <fmt/base.h>
#include <functional>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/STD/string.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <string>
#include <type_traits>

namespace mcs::util
{
  // Wrapper around std::string that takes care of quoting in read and
  // fmt and has the property
  //
  //     read . fmt = identity'
  //
  struct string
  {
    template<typename... Args>
      constexpr explicit string (Args&&...)
        noexcept (std::is_nothrow_constructible_v<std::string, Args&&...>)
      ;

    operator std::string() const;

    auto operator<=> (string const&) const noexcept = default;

  private:
    std::string _str;

    template<typename> friend struct std::hash;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::util::string>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::util::string const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<string>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> string
        ;
  };
}

namespace std
{
  template<>
    struct hash<mcs::util::string>
  {
    auto operator() (mcs::util::string const&) const noexcept -> size_t;

  private:
    hash<decltype (mcs::util::string::_str)> _hash;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<util::string>
  {
    using Type = util::string;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/string.ipp"
