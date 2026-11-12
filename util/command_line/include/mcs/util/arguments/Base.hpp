// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <initializer_list>
#include <mcs/util/command_line/Parsed.hpp>
#include <span>
#include <string_view>
#include <type_traits>

namespace mcs::util::arguments
{
  // Shared read-only access to contiguous argument-pointer storage.
  // Strings must be non-null and null-terminated. Index zero has no
  // special meaning.
  //
  template<typename Storage>
    struct Base
  {
    using Iterator = std::span<char const* const>::iterator;

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t;
    [[nodiscard]] constexpr auto at (std::size_t) const -> char const*;
    [[nodiscard]] constexpr auto begin() const noexcept -> Iterator;
    [[nodiscard]] constexpr auto end() const noexcept -> Iterator;

    // Exact, case-sensitive matching outside nested embeddings.
    // Stops at the first match; validates only markers encountered.
    // An empty list matches nothing. Unclosed embeddings do not
    // throw.
    //
    [[nodiscard]] auto contains_any_of
      ( std::initializer_list<std::string_view>
      ) const -> bool;

    // Whether any top-level argument is exactly "-h" or "--help".
    // Embedded flags are ignored. Stops at the first top-level match.
    //
    [[nodiscard]] auto help_requested() const -> bool;

    [[nodiscard]] auto parse
      ( command_line::Schema const&
      ) const -> command_line::Parsed;

  protected:
    [[nodiscard]] constexpr explicit Base (Storage)
      noexcept (std::is_nothrow_move_constructible_v<Storage>);

    [[nodiscard]] constexpr auto slice
      ( std::size_t
      ) const -> std::span<char const* const>
      ;

    Storage _storage;

  private:
    [[nodiscard]] constexpr auto span
      (
      ) const noexcept -> std::span<char const* const>
      ;
  };
}

#include "detail/Base.ipp"
