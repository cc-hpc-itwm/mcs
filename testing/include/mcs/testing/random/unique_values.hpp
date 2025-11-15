// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <vector>

namespace mcs::testing::random
{
  template<typename Generator, typename T>
    concept is_generator = requires (Generator& generator)
      {
        { generator() } -> std::convertible_to<T>;
      };
  template<typename Sink, typename T>
    concept is_sink = requires (Sink& sink, T x)
      {
        { sink (x) } -> std::convertible_to<void>;
      };

  // Calls `generator()` until `n` distinct values are produced. Calls
  // `sink (x)` for each distinct value `x`.
  //
  template<std::totally_ordered T, typename Generator, typename Sink>
    requires (is_generator<Generator, T> && is_sink<Sink, T>)
    auto unique_values (Generator&, Sink&, std::size_t n) noexcept -> void
    ;

  // Produces a vector of `n` distinct values produced by `generator()`.
  //
  // EXAMPLE:
  //
  //     // Create a vector xs with 100 random int
  //     auto random_value {mcs::testing::random::value<int>{}};
  //     auto xs {mcs::testing::random::unique_values<int> (random_value, 100)};
  //
  template<std::totally_ordered T, typename Generator>
    requires (is_generator<Generator, T>)
    auto unique_values (Generator&, std::size_t n) noexcept -> std::vector<T>
    ;
}

#include "detail/unique_values.ipp"
