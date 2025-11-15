// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <mcs/block_device/block/Count.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>

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

    MCS_UTIL_TUPLISH_ACCESS();
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

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::block_device::block::ID);

#include "detail/ID.ipp"
