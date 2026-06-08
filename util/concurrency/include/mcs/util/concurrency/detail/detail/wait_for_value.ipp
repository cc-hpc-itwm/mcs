// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <atomic>

namespace mcs::util::concurrency::detail
{
  template<is_awaitable_atomic Atomic>
    auto wait_for_value
      ( Atomic& atomic
      , typename Atomic::value_type wanted_value
      ) -> void
  {
  TRY:
    if ( auto const value {atomic.load()}
       ; value != wanted_value
       )
    {
      atomic.wait (value);

      goto TRY;
    }
  }
}
