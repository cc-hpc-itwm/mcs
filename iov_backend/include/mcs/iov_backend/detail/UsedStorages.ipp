// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <utility>

namespace mcs::iov_backend
{
  constexpr auto UsedStorages::Compare::operator()
    ( UsedStorage const& lhs
    , UsedStorage const& rhs
    ) const noexcept -> bool
  {
    return std::less{} (lhs, rhs);
  }

  constexpr auto UsedStorages::Compare::operator()
    ( UsedStorage const& lhs
    , core::memory::Range const& range
    ) const noexcept -> bool
  {
    using core::memory::begin;
    using core::memory::end;

    return ! std::less{} (begin (range), end (lhs._range));
  }
}

namespace mcs::iov_backend
{
  template<typename... Args>
    requires (std::is_constructible_v<UsedStorage, Args...>)
    auto UsedStorages::Constructor::emplace (Args&&... args)
  {
    auto const [pos, inserted]
      { _container.emplace (std::forward<Args> (args)...)
      };

    if (!inserted)
    {
      throw Error::Emplace::Duplicate (*pos);
    }
  }
}

namespace mcs::iov_backend
{
  constexpr auto UsedStorages::Error::Append::RangesAreNotTouching::existing
    (
    ) const noexcept -> core::memory::Range
  {
    return _existing;
  }
  constexpr auto UsedStorages::Error::Append::RangesAreNotTouching::to_append
    (
    ) const noexcept -> core::memory::Range
  {
    return _to_append;
  }
}

namespace mcs::iov_backend
{
  constexpr auto UsedStorages::Error::Construct::RangesAreNotTouching::small
    (
    ) const noexcept -> core::memory::Range
  {
    return _small;
  }
  constexpr auto UsedStorages::Error::Construct::RangesAreNotTouching::large
    (
    ) const noexcept -> core::memory::Range
  {
    return _large;
  }
}
