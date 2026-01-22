// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/lock/detail/wait_for_value.hpp>
#include <mcs/util/lock/queue/FIFO.hpp>

namespace mcs::util::lock::queue
{
  auto FIFO::wait_to_be_served() -> void
  {
    lock::detail::wait_for_value
      ( _next_slot_to_serve
      , _next_available_slot.fetch_add (Ticket {1})
      );
  }
  auto FIFO::next_in_queue() -> void
  {
    _next_slot_to_serve.fetch_add (Ticket {1});
    _next_slot_to_serve.notify_all();
  }
}
