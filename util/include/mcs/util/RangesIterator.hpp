// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <iterator>
#include <optional>
#include <utility>

namespace mcs::util
{
  // Given a sequence of ranges that provide `begin (range)` and `end
  // (range)` for each of the ranges, this iterator iterates the
  // flattened ranges, that is the sequence
  //
  // begin (range_0),... end (range_0), begin (range_1),... end (range_1), ...
  //
  template<typename Ranges>
    struct RangesIterator
  {
    using Iterator = decltype (std::cbegin (std::declval<Ranges>()));
    using Value = decltype (begin (*std::declval<Iterator>()));

    static_assert
      (std::is_same_v<Value, decltype (end (*std::declval<Iterator>()))>);

    using value_type = Value;
    using difference_type = typename Iterator::difference_type;
    using reference = Value const&;
    using pointer = Value const*;
    using iterator_category = std::forward_iterator_tag;

    [[nodiscard]] constexpr explicit RangesIterator (Ranges const&);
    [[nodiscard]] constexpr RangesIterator() = default;

    [[nodiscard]] constexpr auto operator*() const -> reference;
    [[nodiscard]] constexpr auto operator->() const -> pointer;

    constexpr auto operator++() -> RangesIterator&;
    constexpr auto operator++ (int) -> RangesIterator;

    [[nodiscard]] constexpr auto operator==
      ( RangesIterator const&
      ) const -> bool
      ;
    [[nodiscard]] constexpr auto operator!=
      ( RangesIterator const&
      ) const -> bool
      ;

    [[nodiscard]] constexpr operator bool() const;

  private:
    Iterator _pos{};
    Iterator _end{};
    std::optional<Value> _value{};
  };
}

#include "detail/RangesIterator.ipp"
