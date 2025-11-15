// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <chrono>

namespace mcs::util
{
  // Invoke a function and measure execution time.
  //
  // Returns: The result of the function and timer::Ticks.
  //          If the return type is void, then only the timer::Ticks.
  //
  // EXAMPLE:
  //
  //     auto const time {timed ([&] { return; })};
  //     fmt::print ("time {}\n", time);
  //
  // EXAMPLE:
  //
  //     auto const [result, time] {timed ([&] { return 42; })};
  //     fmt::print ("result {}, time {}\n", result, time);
  //
  template< typename Fun
          , long ClockScale = 1'000L
          , typename Clock = std::chrono::steady_clock
          >
    auto timed (Fun&&);
}

#include "detail/timed.ipp"
