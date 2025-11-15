// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <stdexcept>

namespace
{
  template<typename Description, typename T>
    auto queue_get
      ( Description description
      , mcs::iov_backend::detail::Queue<T>& queue
      , typename mcs::iov_backend::detail::Queue<T>::InterruptionContext const& interruption_context
      , std::chrono::time_point<std::chrono::steady_clock> time_point
      ) -> std::optional<T>
  {
    try
    {
      return queue.get (interruption_context, time_point);
    }
    catch (typename mcs::iov_backend::detail::Queue<T>::Error::Get::Timeout const&)
    {
      throw std::runtime_error {fmt::format ("{}: Timeout", description)};
    }
    catch (...)
    {
      return {};
    }
  }
}
