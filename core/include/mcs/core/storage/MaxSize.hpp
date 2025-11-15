// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/read/declare.hpp>
#include <variant>

namespace mcs::core::storage
{
  struct MaxSize
  {
    struct Unlimited{};
    struct Limit
    {
      memory::Size size;
    };

    constexpr MaxSize (Unlimited) noexcept;
    constexpr MaxSize (Limit) noexcept;

    // Returns true if there is no limit on the size.
    //
    constexpr auto is_unlimited() const noexcept -> bool;

    // Pre: !is_unlimited()
    // Returns the limit.
    //
    constexpr auto limit() const -> memory::Size;

  private:
    friend constexpr auto operator>
      ( memory::Size const& wanted_size
      , MaxSize const&
      ) -> bool
      ;
    friend constexpr auto operator- (MaxSize, memory::Size) -> MaxSize;
    friend constexpr auto operator+ (MaxSize, MaxSize) -> MaxSize;
    friend constexpr auto operator< (MaxSize, MaxSize) -> bool;

    std::variant<Unlimited, Limit> _limit;

    constexpr MaxSize (decltype (_limit)) noexcept;

    MCS_UTIL_FMT_ACCESS();
    MCS_SERIALIZATION_ACCESS();

    // \note defined in (some) tests
    friend constexpr auto operator==
      ( MaxSize const&
      , MaxSize const&
      ) -> bool
      ;
  };

  constexpr auto operator>
    ( memory::Size const& wanted_size
    , MaxSize const& max_size
    ) -> bool
    ;

  [[nodiscard]] constexpr auto operator- (MaxSize, memory::Size) -> MaxSize;
  [[nodiscard]] constexpr auto operator+ (MaxSize, MaxSize) -> MaxSize;
  [[nodiscard]] constexpr auto operator< (MaxSize, MaxSize) -> bool;
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE (mcs::core::storage::MaxSize);
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::storage::MaxSize);
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (core::storage::MaxSize);
}

#include "detail/MaxSize.ipp"
