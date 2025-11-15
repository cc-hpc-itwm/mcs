// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <iterator>
#include <mcs/iov_backend/Locations.hpp>
#include <mcs/serialization/STD/vector.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <utility>

namespace mcs::iov_backend
{
  Locations::Locations (ContainerOfLocation locations)
    : _locations {std::move (locations)}
  {}

  auto Locations::begin() const noexcept -> ConstIterator
  {
    return std::begin (_locations);
  }

  auto Locations::end() const noexcept -> ConstIterator
  {
    return std::end (_locations);
  }
}

namespace mcs::iov_backend
{
  auto Locations::ConstIterator::operator*() const -> value_type
  {
    return *_pos;
  }

  auto Locations::ConstIterator::operator++() noexcept -> ConstIterator&
  {
    ++_pos;

    return *this;
  }

  auto Locations::ConstIterator::operator++ (int) noexcept -> ConstIterator
  {
    auto state {*this};
    ++*this;
    return state;
  }

  Locations::ConstIterator::ConstIterator (underlying_iterator pos) noexcept
    : _pos {pos}
  {}
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::iov_backend::Locations
  , _locations
  );
