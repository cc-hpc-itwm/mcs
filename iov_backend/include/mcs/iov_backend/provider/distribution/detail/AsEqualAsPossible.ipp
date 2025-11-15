// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::iov_backend::provider::distribution
{
  [[nodiscard]] constexpr auto operator<
    ( AsEqualAsPossible::StorageIDAndCapacity const& lhs
    , AsEqualAsPossible::StorageIDAndCapacity const& rhs
    ) -> bool
  {
    return rhs.capacity < lhs.capacity;
  }
}
