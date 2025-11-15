// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util
{
  template<long ClockScale, typename Clock>
    auto Timer<ClockScale, Clock>::stop()&& -> timer::Ticks<ClockScale>
  {
    using Duration = std::chrono::duration<long, std::ratio<1L, ClockScale>>;

    return std::chrono::duration_cast<Duration>
      ( Clock::now() - _start
      ).count()
      ;
  }
}
