// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/iov_backend/storage/ID.hpp>
#include <queue>
#include <unordered_map>

namespace mcs::iov_backend::provider::distribution
{
  // Distributes size many bytes as equal as possible over the
  // storages, taking their capacities into account. If some storages
  // have not enough capacity for equal distribution, then they will
  // be filled up and the rest goes equally to the other storages,
  // smaller storages are considered before larger storages.
  //
  // Pre: size < sum (capacities)
  // Post: sum (sizes (ranges)) == size
  //       size (range.at (i)) < capacity.at (i)
  //       the ranges are touching
  //       the ranges are a partition of [0, size)
  //
  struct AsEqualAsPossible
  {
    struct StorageIDAndCapacity
    {
      storage::ID storage_id;
      core::storage::MaxSize capacity;
    };

    [[nodiscard]] auto operator()
      ( core::memory::Size
      , std::priority_queue<StorageIDAndCapacity>
      ) -> std::unordered_map<storage::ID, core::memory::Range>
      ;
  };

  [[nodiscard]] constexpr auto operator<
    ( AsEqualAsPossible::StorageIDAndCapacity const&
    , AsEqualAsPossible::StorageIDAndCapacity const&
    ) -> bool
    ;
}

#include "detail/AsEqualAsPossible.ipp"
