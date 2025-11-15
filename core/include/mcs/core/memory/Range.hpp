// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/Error.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>

namespace mcs::core::memory
{
  // [begin, end)
  struct Range
  {
    friend constexpr auto make_range (Offset, Offset) -> Range;
    friend constexpr auto make_range (Offset, Size) -> Range;

    friend constexpr auto begin (Range const&) -> Offset;
    friend constexpr auto end (Range const&) -> Offset;
    friend constexpr auto size (Range const&) -> Size;

    constexpr auto operator<=> (Range const&) const noexcept = default;

    friend constexpr auto shift (Range, Offset) -> Range;

    struct Error
    {
      struct BeginMustNotBeLargerThanEnd : public mcs::Error
      {
        [[nodiscard]] constexpr auto begin() const noexcept -> Offset;
        [[nodiscard]] constexpr auto end() const noexcept -> Offset;

        MCS_ERROR_COPY_MOVE_DEFAULT (BeginMustNotBeLargerThanEnd);

      private:
        friend struct Range;

        explicit BeginMustNotBeLargerThanEnd (Offset, Offset) noexcept;

        Offset _begin;
        Offset _end;
      };
    };

  private:
    constexpr explicit Range (Offset, Offset);

    Offset _begin;
    Offset _end;

    MCS_UTIL_FMT_ACCESS();
    MCS_SERIALIZATION_ACCESS();
  };

  template<std::integral I>
    [[nodiscard]] constexpr auto make_range (I, I) -> Range;

  [[nodiscard]] constexpr auto make_range (Offset, Offset) -> Range;
  [[nodiscard]] constexpr auto make_range (Offset, Size) -> Range;

  [[nodiscard]] constexpr auto begin (Range const&) -> Offset;
  [[nodiscard]] constexpr auto end (Range const&) -> Offset;
  [[nodiscard]] constexpr auto size (Range const&) -> Size;

  [[nodiscard]] constexpr auto shift (Range, Offset) -> Range;

  template<typename T>
    [[nodiscard]] constexpr auto select (std::span<T>, Range const&);
}

namespace fmt
{
  template<>
    MCS_UTIL_FMT_DECLARE (mcs::core::memory::Range);
}

namespace mcs::serialization
{
  template<> MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
    ( core::memory::Range
    );
}

namespace mcs::util::read
{
  template<>
    MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION (core::memory::Range);
}

#include "detail/Range.ipp"
