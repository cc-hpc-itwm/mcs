// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

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
        ~MustNotBeZero() override;
        MustNotBeZero (MustNotBeZero const&) = default;
        MustNotBeZero (MustNotBeZero&&) noexcept = default;
        auto operator= (MustNotBeZero const&) -> MustNotBeZero& = default;
        auto operator= (MustNotBeZero&&) noexcept  -> MustNotBeZero& = default;

      private:
        friend struct Size;

        MustNotBeZero();
      };
    };

  private:
    underlying_type _value {0u};

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;
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

namespace fmt
{
  template<>
    struct formatter<mcs::block_device::block::Size>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::block_device::block::Size const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::block_device::block::Size>
  {
    using Type = mcs::block_device::block::Size;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::block_device::block::Size>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::block_device::block::Size
        ;
  };
}

#include "detail/Size.ipp"
