// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <cstdint>
#include <fmt/base.h>
#include <functional>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>

namespace mcs::core::storage
{
  struct ID
  {
    [[nodiscard]] constexpr ID() noexcept = default;

    constexpr auto operator++() noexcept -> ID&;
    constexpr auto operator<=> (ID const&) const noexcept = default;

  private:
    using underlying_type = std::uintmax_t;

    underlying_type _value {0u};

    [[nodiscard]] constexpr explicit ID (underlying_type) noexcept;

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct std::hash;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::ID>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::ID const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace std
{
  template<>
    struct hash<mcs::core::storage::ID>
  {
    auto operator() (mcs::core::storage::ID) const noexcept -> size_t;

  private:
    hash<mcs::core::storage::ID::underlying_type> _hash;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::storage::ID>
  {
    using Type = core::storage::ID;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<core::storage::ID>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::storage::ID
        ;
  };
}

#include "detail/ID.ipp"
