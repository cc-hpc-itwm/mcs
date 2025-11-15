// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <mcs/iov_backend/provider/distribution/AsEqualAsPossible.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/util/distribution/equal.hpp>
#include <queue>
#include <ranges>
#include <vector>

namespace mcs::iov_backend::provider::distribution
{
  auto AsEqualAsPossible::operator()
    ( core::memory::Size size
    , std::priority_queue<StorageIDAndCapacity> capacities
    ) -> std::unordered_map<storage::ID, core::memory::Range>
  {
    auto ranges {std::unordered_map<storage::ID, core::memory::Range>{}};

    // Start at the absolute position
    auto begin {core::memory::make_offset (0)};

    while (!capacities.empty())
    {
      auto const& [storage_id, capacity] {capacities.top()};
      auto const pop
        { nonstd::make_scope_success_that_dies_on_exception
          ( "mcs::iov_backend::provider::AsEqualAsPossible::pop"
          , [&]
            {
              capacities.pop();
            }
          )
        };

      // The size that would be perfect for an equal distribution
      auto const wanted_size
        { core::memory::make_size
          ( util::distribution::equal::size
            ( size_cast<decltype (capacities.size())> (size)
            , capacities.size()
            , decltype (capacities.size()) {0}
            )
          )
        };

      auto const range
        { ranges.emplace
          ( storage_id
          , core::memory::make_range
            ( begin
            , wanted_size > capacity  // if the wanted size does not fit, ...
              ? capacity.limit()      // ... then take as much as possible
              : wanted_size
            )
          ).first->second
        };

      // Later absolute positions need to take into account the
      // already consumed part ...
      begin += core::memory::size (range);
      // ... and distribute less elements
      size -= core::memory::size (range);
    }

    return ranges;
  }
}
