// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/block_device/block/ID.hpp>
#include <mcs/util/tuplish/access.hpp>
#include <mcs/util/tuplish/declare.hpp>

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

        MCS_ERROR_COPY_MOVE_DEFAULT (BeginMustBeSmallerThanEnd);

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

    MCS_UTIL_TUPLISH_ACCESS();
  };

  [[nodiscard]] constexpr auto UNSAFE_make_range (ID, ID) noexcept -> Range;
  [[nodiscard]] constexpr auto make_range (ID, ID) -> Range;
  [[nodiscard]] constexpr auto make_range (ID, Count) -> Range;
  [[nodiscard]] constexpr auto begin (Range) noexcept -> ID;
  [[nodiscard]] constexpr auto end (Range) noexcept -> ID;
  [[nodiscard]] constexpr auto size (Range) noexcept -> Count;
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION (mcs::block_device::block::Range);

#include "detail/Range.ipp"
