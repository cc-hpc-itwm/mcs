// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/iov_backend/UsedStorages.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <memory>
#include <utility>

namespace mcs::iov_backend
{
  UsedStorages::ConstIterator::ConstIterator
    ( underlying_iterator pos
    , underlying_iterator end
    ) noexcept
      : _pos {pos}
      , _end {end}
  {}

  auto UsedStorages::ConstIterator::operator->
    (
    ) const noexcept -> value_type const*
  {
    return std::addressof (*_pos);
  }
  auto UsedStorages::ConstIterator::operator*
    (
    ) const noexcept -> value_type const&
  {
    return *_pos;
  }
  auto UsedStorages::ConstIterator::operator++() noexcept -> ConstIterator&
  {
    ++_pos;
    return *this;
  }
  auto UsedStorages::ConstIterator::operator++ (int) noexcept -> ConstIterator
  {
    auto state {*this};
    ++*this;
    return state;
  }

  auto UsedStorages::ConstIterator::operator==
    ( ConstIterator const& other
    ) const noexcept -> bool
  {
    return _pos == other._pos;
  }
  auto UsedStorages::ConstIterator::operator!=
    ( ConstIterator const& other
    ) const noexcept -> bool
  {
    return _pos != other._pos;
  }
}

namespace mcs::iov_backend
{
  UsedStorages::Constructor::Error::Emplace::Duplicate::Duplicate
    ( UsedStorage used_storage
    )
      : mcs::Error
        { fmt::format
          ( "UsedStorage::Constructor::Emplace::Duplicate: {}"
          , used_storage
          )
        }
      , _used_storage {std::move (used_storage)}
  {}
  UsedStorages::Constructor::Error::Emplace::Duplicate::~Duplicate() = default;
  auto UsedStorages::Constructor::Error::Emplace::Duplicate::used_storage
    (
    ) const -> UsedStorage
  {
    return _used_storage;
  }

  UsedStorages::Error::Construct::RangesAreNotTouching::RangesAreNotTouching
    ( core::memory::Range small
    , core::memory::Range large
    )
      : mcs::Error
        { fmt::format
          ( "UsedStorages::Construct::RangesAreNotTouching: "
            "Ranges must be consecutive. There is a gap between {} and {}."
          , small
          , large
          )
        }
      , _small {small}
      , _large {large}
  {}
  UsedStorages::Error::Construct::RangesAreNotTouching::~RangesAreNotTouching()
    = default
    ;

  UsedStorages::Error::Append::RangesAreNotTouching::RangesAreNotTouching
    ( core::memory::Range existing
    , core::memory::Range to_append
    )
      : mcs::Error
        { fmt::format
          ( "UsedStorages::Append::RangesAreNotTouching: "
            "Ranges must be consecutive. Existing: {}, to append: {}."
          , existing
          , to_append
          )
        }
      , _existing {existing}
      , _to_append {to_append}
  {}
  UsedStorages::Error::Append::RangesAreNotTouching::~RangesAreNotTouching()
    = default
    ;

  UsedStorages::Error::Range::Empty::Empty()
    : mcs::Error {"UsedStorages::range(): Empty."}
  {}
  UsedStorages::Error::Range::Empty::~Empty() = default;
}

namespace mcs::iov_backend
{
  UsedStorages::UsedStorages (Constructor constructor)
    : _container {std::move (constructor._container)}
  {
    struct RangesWithGap
    {
      core::memory::Range small;
      core::memory::Range large;
    };

    auto const ranges_with_gap
      { [&]() -> std::optional<RangesWithGap>
        {
          // c++23 adjacent_view
          for ( auto used_storage {std::cbegin (_container)}
              ; used_storage != std::cend (_container)
                && std::next (used_storage) != std::cend (_container)
              ; ++used_storage
              )
          {
            using core::memory::begin;
            using core::memory::end;

            auto const small {used_storage->_range};
            auto const large {std::next (used_storage)->_range};

            if (end (small) != begin (large))
            {
              return RangesWithGap {small, large};
            }
          }

          return {};
        }
      };

    if (auto const gap {std::invoke (ranges_with_gap)})
    {
      throw Error::Construct::RangesAreNotTouching {gap->small, gap->large};
    }
  }

  auto UsedStorages::append (UsedStorages used_storages) -> void
  {
    auto const existing {range()};
    auto const to_append {used_storages.range()};

    using core::memory::begin;
    using core::memory::end;

    if (end (existing) != begin (to_append))
    {
      throw Error::Append::RangesAreNotTouching {existing, to_append};
    }

    // \todo Is merge the right thing to do!? Should consecutive
    // ranges be joined?
    _container.merge (used_storages._container);
  }

  auto UsedStorages::begin() const noexcept -> ConstIterator
  {
    return {std::begin (_container), std::end (_container)};
  }
  auto UsedStorages::end() const noexcept -> ConstIterator
  {
    return {std::end (_container), std::end (_container)};
  }

  auto UsedStorages::lower_bound
    ( core::memory::Range range
    ) const noexcept -> ConstIterator
  {
    return {_container.lower_bound (range), std::end (_container)};
  }

  auto UsedStorages::empty() const noexcept -> bool
  {
    return _container.empty();
  }

  auto UsedStorages::range() const -> core::memory::Range
  {
    if (empty())
    {
      throw Error::Range::Empty{};
    }

    return core::memory::make_range
      ( core::memory::begin (std::cbegin (_container)->_range)
      , core::memory::end (std::crbegin (_container)->_range)
      );
  }
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::iov_backend::UsedStorages
  , _container
  );

namespace mcs::iov_backend
{
  UsedStorages::UsedStorages (Container container) noexcept
    : _container {std::move (container)}
  {}
}
