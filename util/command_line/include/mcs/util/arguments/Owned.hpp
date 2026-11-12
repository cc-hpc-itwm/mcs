// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/util/arguments/Base.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/not_null.hpp>
#include <vector>

namespace mcs::util::arguments
{
  // Owns the pointer array, not its strings. Mutations may invalidate
  // iterators and views. The strings must outlive their use through
  // this owner and any derived views.
  //
  struct Owned : Base<std::vector<char const*>>
  {
    [[nodiscard]] explicit Owned (View);

    // Append a borrowed, null-terminated string.
    //
    auto operator+= (util::not_null<char const>) -> Owned&;

    // Remove all content-equal top-level occurrences, preserving
    // embeddings and remaining order. Uses the same marker rules as
    // contains_any_of.
    //
    auto operator-= (util::not_null<char const>) -> Owned&;

    [[nodiscard]] auto view() const noexcept -> View;
    [[nodiscard]] auto from (std::size_t) const -> View;
  };
}
