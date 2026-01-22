// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <fmt/base.h>
#include <functional>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <span>

namespace mcs::core::memory
{
  struct Offset;
  struct Range;

  struct Size
  {
    using underlying_type = std::uintmax_t;

    template<std::integral I>
      friend constexpr auto make_size
        ( I
        ) noexcept (std::unsigned_integral<I>) -> Size
      ;

    constexpr auto operator<=> (Size const&) const noexcept = default;

    constexpr auto operator+= (Size const&) noexcept -> Size&;
    constexpr auto operator-= (Size const&) noexcept -> Size&;

    friend constexpr auto operator+
      ( Size const&
      , Size const&
      ) noexcept -> Size
      ;
    friend constexpr auto operator/
      ( Size const&
      , Size const&
      ) -> Size::underlying_type
      ;
    friend constexpr auto operator%
      ( Size const&
      , Size const&
      ) -> Size::underlying_type
      ;
    template<std::unsigned_integral T>
      friend constexpr auto operator*
        ( T const&
        , Size const&
        ) noexcept -> Size
      ;
    template<std::unsigned_integral T>
      friend constexpr auto operator*
        ( Size const&
        , T const&
        ) noexcept -> Size
      ;
    friend constexpr auto operator+
      ( Offset const& lhs
      , Size const& rhs
      ) -> Offset
      ;
    friend constexpr auto operator-
      ( Offset const& lhs
      , Size const& rhs
      ) -> Offset
      ;
    friend constexpr auto operator- (Size, Size) -> Size;
    friend constexpr auto divru
      ( Size const&
      , Size const&
      ) -> Size::underlying_type
      ;

    friend constexpr auto make_range (Offset, Size) -> Range;

    template<typename T>
      friend constexpr auto select (std::span<T>, Range const&);

    template<std::unsigned_integral I>
      requires (sizeof (I) >= sizeof (Size::underlying_type))
      friend auto size_cast (Size const&) -> I;

  private:
    underlying_type _value {0u};

    template<std::unsigned_integral I>
      [[nodiscard]] constexpr explicit Size (I) noexcept;
    template<std::signed_integral I>
      [[nodiscard]] constexpr explicit Size (I);

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct std::hash;
    template<typename> friend struct serialization::Implementation;
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_size
      ( I
      ) noexcept (std::unsigned_integral<I>) -> Size
    ;

  [[nodiscard]] constexpr auto operator+
    ( Size const&
    , Size const&
    ) noexcept -> Size
    ;
  [[nodiscard]] constexpr auto operator/
    ( Size const&
    , Size const&
    ) -> Size::underlying_type
    ;
  [[nodiscard]] constexpr auto operator%
    ( Size const&
    , Size const&
    ) -> Size::underlying_type
    ;
  template<std::unsigned_integral T>
    [[nodiscard]] constexpr auto operator*
      ( T const&
      , Size const&
      ) noexcept -> Size
    ;
  template<std::unsigned_integral T>
    [[nodiscard]] constexpr auto operator*
      ( Size const&
      , T const&
      ) noexcept -> Size
    ;
  [[nodiscard]] constexpr auto operator+
    ( Offset const& lhs
    , Size const& rhs
    ) -> Offset
    ;

  [[nodiscard]] constexpr auto operator- (Size, Size) -> Size;

  [[nodiscard]] constexpr auto make_range (Offset, Size) -> Range;

  template<typename T>
    [[nodiscard]] constexpr auto select (std::span<T>, Range const&);

  template<std::unsigned_integral I>
    requires (sizeof (I) >= sizeof (Size::underlying_type))
    [[nodiscard]] auto size_cast (Size const&) -> I;
}

namespace std
{
  template<>
    struct hash<mcs::core::memory::Size>
  {
    auto operator() (mcs::core::memory::Size) const noexcept -> size_t;

  private:
    hash<mcs::core::memory::Size::underlying_type> _hash;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::memory::Size>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::memory::Size const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::memory::Size>
  {
    using Type = core::memory::Size;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<core::memory::Size>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::memory::Size
        ;
  };
}

#include "detail/Size.ipp"
