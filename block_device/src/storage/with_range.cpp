// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/block_device/storage/with_range.hpp>
#include <utility>

namespace mcs::block_device::storage
{
  auto with_range
    ( Storage storage
    , core::memory::Range range
    ) noexcept -> Storage
  {
    storage._range = range;
    return storage;
  }
  auto with_range
    ( Storage storage
    , core::memory::Offset begin
    , core::memory::Offset end
    ) noexcept -> Storage
  {
    return with_range
      ( storage
      , core::memory::make_range (begin, end)
      );
  }
}
