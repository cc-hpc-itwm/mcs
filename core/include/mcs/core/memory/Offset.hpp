// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/hash/access.hpp>
#include <mcs/util/hash/declare.hpp>
#include <mcs/util/read/declare.hpp>
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

    MCS_UTIL_FMT_ACCESS();
    MCS_UTIL_HASH_ACCESS();
    MCS_SERIALIZATION_ACCESS();
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_offset
      ( I
      ) noexcept (std::unsigned_integral<I>) -> Offset
    ;

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
    MCS_UTIL_HASH_DECLARE_VIA_HASH_OF_UNDERLYING_TYPE
      ( mcs::core::memory::Offset
      );
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE (mcs::core::memory::Offset);
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::memory::Offset
      );
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (core::memory::Offset);
}

#include "detail/Offset.ipp"
