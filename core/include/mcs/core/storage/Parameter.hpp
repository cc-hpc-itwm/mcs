// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstddef>
#include <fmt/base.h>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>
#include <vector>

namespace mcs::core::storage
{
  struct Parameter
  {
    template<serialization::is_serializable T>
      friend auto make_parameter (T const&) -> Parameter;

    template<serialization::is_serializable T> auto as() const -> T;

#ifdef __cpp_lib_constexpr_vector
    constexpr
#endif
              auto operator<=> (Parameter const&) const noexcept = default;

  private:
    struct Make{};

    template<serialization::is_serializable T>
      explicit Parameter (Make, T const&);

    std::vector<std::byte> _blob;

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;

    explicit Parameter (std::vector<std::byte>);
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::Parameter>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::Parameter const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::Parameter>
  {
    using Type = mcs::core::storage::Parameter;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::Parameter>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::Parameter
        ;
  };
}

#include "detail/Parameter.ipp"
