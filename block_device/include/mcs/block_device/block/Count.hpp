// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <fmt/base.h>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::block_device::block
{
  struct ID;

  struct Count
  {
    using underlying_type = std::uintmax_t;

    template<std::unsigned_integral I>
      [[nodiscard]] constexpr explicit Count (I) noexcept;
    template<std::signed_integral I>
      [[nodiscard]] constexpr explicit Count (I);

    constexpr auto operator<=> (Count const&) const noexcept = default;

    constexpr auto operator+= (Count const&) noexcept -> Count&;
    constexpr auto operator-= (Count const&) noexcept -> Count&;

    friend constexpr auto operator+
      ( Count const&
      , Count const&
      ) noexcept -> Count
      ;
    friend constexpr auto operator-
      ( Count const&
      , Count const&
      ) noexcept -> Count
      ;
    friend constexpr auto operator*
      ( Count const&
      , Size const&
      ) noexcept -> core::memory::Size
      ;
    friend constexpr auto operator+
      ( ID const&
      , Count const&
      ) noexcept -> ID
      ;

  private:
    friend struct ID;

    underlying_type _value {0u};

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_count
      ( I
      ) noexcept (std::unsigned_integral<I>) -> Count
    ;

  [[nodiscard]] constexpr auto operator+
    ( Count const&
    , Count const&
    ) noexcept -> Count
    ;
  [[nodiscard]] constexpr auto operator-
    ( Count const&
    , Count const&
    ) noexcept -> Count
    ;
  [[nodiscard]] constexpr auto operator*
    ( Count const&
    , Size const&
    ) noexcept -> core::memory::Size
    ;
  [[nodiscard]] constexpr auto operator+
    ( ID const&
    , Count const&
    ) noexcept -> ID
    ;
}

namespace fmt
{
  template<>
    struct formatter<mcs::block_device::block::Count>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::block_device::block::Count const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::block_device::block::Count>
  {
    using Type = mcs::block_device::block::Count;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::block_device::block::Count>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::block_device::block::Count
        ;
  };
}

#include "detail/Count.ipp"
