// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <fmt/base.h>
#include <mcs/block_device/block/Count.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::block_device::block
{
  struct ID
  {
    using underlying_type = std::uintmax_t;

    template<std::unsigned_integral I>
      [[nodiscard]] constexpr explicit ID (I) noexcept;
    template<std::signed_integral I>
      [[nodiscard]] constexpr explicit ID (I);

    constexpr auto operator<=> (ID const&) const noexcept = default;

    constexpr auto operator+= (Count const&) noexcept -> ID&;
    constexpr auto operator++() noexcept -> ID&;

    friend constexpr auto operator+
      ( ID const&
      , Count const&
      ) noexcept -> ID
      ;
    friend constexpr auto operator-
      ( ID const&
      , ID const&
      ) noexcept -> Count
      ;

  private:
    underlying_type _value {0u};

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_id
      ( I
      ) noexcept (std::unsigned_integral<I>) -> ID
    ;

  [[nodiscard]] constexpr auto operator+
    ( ID const&
    , Count const&
    ) noexcept -> ID
    ;
  [[nodiscard]] constexpr auto operator-
    ( ID const&
    , ID const&
    ) noexcept -> Count
    ;
}

namespace fmt
{
  template<>
    struct formatter<mcs::block_device::block::ID>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::block_device::block::ID const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::block_device::block::ID>
  {
    using Type = mcs::block_device::block::ID;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::block_device::block::ID>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::block_device::block::ID
        ;
  };
}

#include "detail/ID.ipp"
