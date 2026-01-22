// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <fmt/base.h>
#include <functional>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <span>
#include <sys/types.h>

namespace mcs::core::memory
{
  struct Range;

  struct Offset
  {
    using underlying_type = std::uintmax_t;

    template<std::integral I>
      friend constexpr auto make_offset
        ( I
        ) noexcept (std::unsigned_integral<I>) -> Offset
      ;

    constexpr auto operator<=> (Offset const&) const noexcept = default;

    constexpr auto operator+= (Size const&) -> Offset&;

    friend constexpr auto operator- (Offset) -> Offset;

    friend constexpr auto operator-
      ( Offset const&
      , Offset const&
      ) -> Size
      ;
    friend constexpr auto operator+
      ( Offset const&
      , Size const&
      ) -> Offset
      ;
    friend constexpr auto operator+ (Offset, Offset) -> Offset;
    friend constexpr auto operator-
      ( Offset const&
      , Size const&
      ) -> Offset
      ;

    friend constexpr auto make_range (Offset, Size) -> Range;

    template<typename T>
      friend constexpr auto select
        ( std::span<T>
        , Range const&
        );

    template<typename T>
      friend constexpr auto operator+
        ( T*
        , Offset
        ) -> T*
      ;

    friend constexpr auto make_off_t (Offset) -> off_t;

    template<std::unsigned_integral I>
      requires (sizeof (I) >= sizeof (Offset::underlying_type))
      friend auto offset_cast (Offset const&) -> I;

  private:
    underlying_type _value {0u};

    template<std::unsigned_integral I>
      [[nodiscard]] constexpr explicit Offset (I) noexcept;
    template<std::signed_integral I>
      [[nodiscard]] constexpr explicit Offset (I);

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct std::hash;
    template<typename> friend struct serialization::Implementation;
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_offset
      ( I
      ) noexcept (std::unsigned_integral<I>) -> Offset
    ;

  [[nodiscard]] constexpr auto operator- (Offset) -> Offset;

  [[nodiscard]] constexpr auto operator-
      ( Offset const&
      , Offset const&
      ) -> Size
      ;
  [[nodiscard]] constexpr auto operator+
      ( Offset const&
      , Size const&
      ) -> Offset
      ;
  [[nodiscard]] constexpr auto operator+ (Offset, Offset) -> Offset;
  [[nodiscard]] constexpr auto operator-
      ( Offset const&
      , Size const&
      ) -> Offset
      ;

  [[nodiscard]] constexpr auto make_range (Offset, Size) -> Range;

  template<typename T>
    [[nodiscard]] constexpr auto select (std::span<T>, Range const&);

  template<typename T>
    [[nodiscard]] constexpr auto operator+
      ( T*
      , Offset
      ) -> T*
    ;

  [[nodiscard]] constexpr auto make_off_t (Offset) -> off_t;
}

namespace std
{
  template<>
    struct hash<mcs::core::memory::Offset>
  {
    auto operator() (mcs::core::memory::Offset) const noexcept -> size_t;

  private:
    hash<mcs::core::memory::Offset::underlying_type> _hash;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::memory::Offset>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::memory::Offset const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::memory::Offset>
  {
    using Type = core::memory::Offset;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<core::memory::Offset>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::memory::Offset
        ;
  };
}

#include "detail/Offset.ipp"
