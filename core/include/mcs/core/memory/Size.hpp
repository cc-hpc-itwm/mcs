// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/declare.hpp>
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

    MCS_UTIL_FMT_ACCESS();
    MCS_UTIL_HASH_ACCESS();
    MCS_SERIALIZATION_ACCESS();
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
    MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
      ( mcs::core::memory::Size
      );
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE (mcs::core::memory::Size);
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( core::memory::Size
    );
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (core::memory::Size);
}

#include "detail/Size.ipp"
