// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::block_device::meta_data
{
  constexpr auto Blocks::Error::BlockNotInAnyStorage::block_id() const noexcept
  {
    return _block_id;
  }

  constexpr auto Blocks::UsedStorageCompare::operator()
    ( Blocks::UsedStorage const& lhs
    , Blocks::UsedStorage const& rhs
    ) const noexcept -> bool
  {
    // \note It is not enough to compare begin only or end only:
    // Addition of storages ensures there are never ranges with the
    // same begin or the same end. However, during removal there might
    // be intermediate phases where two ranges with the same begin
    // and/or two ranges with the same end exist.
    return std::less{} (lhs.range, rhs.range);
  }
  constexpr auto Blocks::UsedStorageCompare::operator()
    ( block::ID const& lhs
    , Blocks::UsedStorage const& rhs
    ) const noexcept -> bool
  {
    return std::less{} (lhs, end (rhs.range));
  }

  auto constexpr Blocks::block_size() const noexcept -> block::Size
  {
    return _block_size;
  }
}
