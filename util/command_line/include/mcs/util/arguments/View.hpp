// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/util/arguments/Base.hpp>
#include <span>

namespace mcs::util::arguments
{
  // The pointer array and its strings must outlive this view and any
  // derived views.
  //
  struct View : Base<std::span<char const* const>>
  {
    [[nodiscard]] constexpr explicit View
      ( char const* const*
      , std::size_t
      ) noexcept;

    // Includes the argument at offset. An offset equal to size is
    // valid. Invalid offsets throw std::out_of_range.
    //
    [[nodiscard]] constexpr auto from (std::size_t offset) const -> View;
  };
}

#include "detail/View.ipp"
