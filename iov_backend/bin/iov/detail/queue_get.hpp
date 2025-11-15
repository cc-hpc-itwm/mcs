// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <chrono>
#include <mcs/iov_backend/detail/Queue.hpp>
#include <optional>

namespace
{
  // Throws on timeout, returns nothing when interrupted
  template<typename Description, typename T>
    auto queue_get
      ( Description
      , mcs::iov_backend::detail::Queue<T>&
      , typename mcs::iov_backend::detail::Queue<T>::InterruptionContext const&
        = typename mcs::iov_backend::detail::Queue<T>::InterruptionContext{}
      , std::chrono::time_point<std::chrono::steady_clock>
        = std::chrono::steady_clock::now() + std::chrono::milliseconds (15'000)
      ) -> std::optional<T>
    ;
}

#include "detail/queue_get.ipp"
