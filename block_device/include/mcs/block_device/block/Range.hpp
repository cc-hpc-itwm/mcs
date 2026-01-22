// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::block_device::block
{
  struct Range
  {
    constexpr Range (ID, ID);

    friend constexpr auto UNSAFE_make_range (ID, ID) noexcept -> Range;
    friend constexpr auto make_range (ID, ID) -> Range;
    friend constexpr auto make_range (ID, Count) -> Range;
    friend constexpr auto begin (Range) noexcept -> ID;
    friend constexpr auto end (Range) noexcept -> ID;

    constexpr auto operator<=> (Range const&) const noexcept = default;

    [[nodiscard]] constexpr auto is_extended_by (Range const&) const noexcept -> bool;
    constexpr auto extend_by (Range const&) noexcept -> Range&;

    struct Error
    {
      struct BeginMustBeSmallerThanEnd : public mcs::Error
      {
        [[nodiscard]] constexpr auto begin() const noexcept -> ID;
        [[nodiscard]] constexpr auto end() const noexcept -> ID;

        ~BeginMustBeSmallerThanEnd() override;
        BeginMustBeSmallerThanEnd (BeginMustBeSmallerThanEnd const&) = default;
        BeginMustBeSmallerThanEnd (BeginMustBeSmallerThanEnd&&) noexcept = default;
        auto operator= (BeginMustBeSmallerThanEnd const&) -> BeginMustBeSmallerThanEnd& = default;
        auto operator= (BeginMustBeSmallerThanEnd&&) noexcept  -> BeginMustBeSmallerThanEnd& = default;

      private:
        friend struct Range;

        BeginMustBeSmallerThanEnd (ID, ID);

        ID _begin;
        ID _end;
      };
    };

  private:
    struct UNSAFE{};
    constexpr Range (UNSAFE, ID, ID) noexcept;

    ID _begin;
    ID _end;

    template<typename, typename, typename> friend struct fmt::formatter;
    template<typename> friend struct serialization::Implementation;
    template<typename> friend struct util::read::Read;
  };

  [[nodiscard]] constexpr auto UNSAFE_make_range (ID, ID) noexcept -> Range;
  [[nodiscard]] constexpr auto make_range (ID, ID) -> Range;
  [[nodiscard]] constexpr auto make_range (ID, Count) -> Range;
  [[nodiscard]] constexpr auto begin (Range) noexcept -> ID;
  [[nodiscard]] constexpr auto end (Range) noexcept -> ID;
  [[nodiscard]] constexpr auto size (Range) noexcept -> Count;
}

namespace fmt
{
  template<>
    struct formatter<mcs::block_device::block::Range>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::block_device::block::Range const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::block_device::block::Range>
  {
    using Type = mcs::block_device::block::Range;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::block_device::block::Range>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::block_device::block::Range
        ;
  };
}

#include "detail/Range.ipp"
