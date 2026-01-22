// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <type_traits>

namespace mcs::fuse::content::storage
{
  // Files are composed of "parts". Each part covers a sub-range in a
  // segment.
  //
  struct Part
  {
    [[nodiscard]] constexpr Part
      ( core::memory::Range
      , core::storage::segment::ID
      ) noexcept
      ;

    [[nodiscard]] constexpr auto range
      (
      ) const noexcept -> core::memory::Range
      ;
    [[nodiscard]] constexpr auto segment_id
      (
      ) const noexcept -> core::storage::segment::ID
      ;

    // Change the end of the range to end.
    //
    constexpr auto crop (core::memory::Offset end) -> void;

  private:
    core::memory::Range _range;
    core::storage::segment::ID _segment_id;
  };
  static_assert (std::is_trivially_copyable_v<Part>);
}

namespace fmt
{
  template<>
    struct formatter<mcs::fuse::content::storage::Part>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::fuse::content::storage::Part const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

#include "detail/Part.ipp"
