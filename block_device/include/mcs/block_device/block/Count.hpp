// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <mcs/block_device/block/Size.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>

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

    MCS_UTIL_TUPLISH_ACCESS();
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

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::block_device::block::Count);

#include "detail/Count.ipp"
