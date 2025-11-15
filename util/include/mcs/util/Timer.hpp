// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <chrono>
#include <mcs/util/timer/Ticks.hpp>

namespace mcs::util
{
  template< long ClockScale = 1'000L
          , typename Clock = std::chrono::steady_clock
          >
    struct Timer
  {
    auto stop()&& -> timer::Ticks<ClockScale>;

  private:
    decltype (Clock::now()) _start {Clock::now()};
  };
}

#include "detail/Timer.ipp"
