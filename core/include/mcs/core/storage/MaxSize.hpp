// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <compare>
#include <fmt/base.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
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

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;

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
    struct formatter<mcs::core::storage::MaxSize>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::MaxSize const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::storage::MaxSize>
  {
    using Type = core::storage::MaxSize;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<core::storage::MaxSize>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::storage::MaxSize
        ;
  };
}

#include "detail/MaxSize.ipp"
