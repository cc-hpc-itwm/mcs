// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <mcs/util/Timer.hpp>
#include <type_traits>
#include <utility>

namespace mcs::util
{
  template<typename Fun, long ClockScale, typename Clock>
    auto timed (Fun&& fun)
  {
    auto timer {Timer<ClockScale, Clock>{}};

    if constexpr (std::is_void_v<std::invoke_result_t<Fun>>)
    {
      std::invoke (std::forward<Fun> (fun));

      return std::move (timer).stop();
    }
    else
    {
      auto result {std::invoke (std::forward<Fun> (fun))};

      return std::make_tuple (std::move (result), std::move (timer).stop());
    }
  }
}
