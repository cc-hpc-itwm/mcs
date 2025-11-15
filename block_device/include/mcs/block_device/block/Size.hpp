// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <mcs/Error.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>

namespace mcs::block_device::block
{
  struct Size;
}

namespace mcs::core::memory
{
  [[nodiscard]] constexpr auto make_size
    ( block_device::block::Size const&
    ) noexcept -> Size;
}

namespace mcs::block_device::block
{
  struct Count;

  struct Size
  {
    using underlying_type = std::uintmax_t;

    template<std::unsigned_integral I>
      [[nodiscard]] constexpr explicit Size (I);
    template<std::signed_integral I>
      [[nodiscard]] constexpr explicit Size (I);

    friend constexpr auto operator/
      ( core::memory::Size
      , Size
      ) noexcept -> core::memory::Size::underlying_type
      ;
    friend constexpr auto operator*
      ( Count const&
      , Size const&
      ) noexcept -> core::memory::Size
      ;

    friend constexpr auto core::memory::make_size
      ( Size const&
      ) noexcept -> core::memory::Size;

    struct Error
    {
      struct MustNotBeZero : public mcs::Error
      {
        MCS_ERROR_COPY_MOVE_DEFAULT (MustNotBeZero);

      private:
        friend struct Size;

        MustNotBeZero();
      };
    };

  private:
    underlying_type _value {0u};

    MCS_UTIL_TUPLISH_ACCESS();
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_size (I) -> Size;

  [[nodiscard]] constexpr auto operator/
    ( core::memory::Size
    , Size
    ) noexcept -> core::memory::Size::underlying_type
    ;
  [[nodiscard]] constexpr auto operator*
    ( Count const&
    , Size const&
    ) noexcept -> core::memory::Size
    ;
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::block_device::block::Size);

#include "detail/Size.ipp"
