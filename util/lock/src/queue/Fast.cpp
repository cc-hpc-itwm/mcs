// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/lock/queue/Fast.hpp>

namespace mcs::util::lock::queue
{
  auto Fast::wait_to_be_served() -> void
  {
    _guard.lock();
  }
  auto Fast::next_in_queue() -> void
  {
    _guard.unlock();
  }
}
