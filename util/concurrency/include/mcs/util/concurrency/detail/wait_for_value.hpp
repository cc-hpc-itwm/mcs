// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>

namespace mcs::util::concurrency::detail
{
  template<typename Atomic>
    concept is_awaitable_atomic = requires
        ( Atomic const& atomic
        , typename Atomic::value_type value
        )
      {
        { atomic.load()
        } -> std::convertible_to<typename Atomic::value_type>
        ;
        { atomic.wait (value)
        } -> std::convertible_to<void>
        ;
      };

  // Wait until a atomic contains a wanted value.
  //
  // EXAMPLE:
  //
  //     auto a {std::atomic<int> {0}};
  //     // spawn some asynchronous operation on a
  //     wait_for_value (a, 5);
  //
  template<is_awaitable_atomic Atomic>
    auto wait_for_value (Atomic&, typename Atomic::value_type) -> void;
}

#include "detail/wait_for_value.ipp"
