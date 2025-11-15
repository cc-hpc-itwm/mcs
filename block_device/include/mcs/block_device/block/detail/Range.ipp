// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/tuplish/define.hpp>

MCS_UTIL_TUPLISH_DEFINE_FMT_READ2
  ( "mcs::block_device::block::Range "
  , mcs::block_device::block::Range
  , _begin
  , _end
  );

namespace mcs::block_device::block
{
  constexpr auto Range::Error::BeginMustBeSmallerThanEnd::begin
    (
    ) const noexcept -> ID
  {
    return _begin;
  }
  constexpr auto Range::Error::BeginMustBeSmallerThanEnd::end
    (
    ) const noexcept -> ID
  {
    return _end;
  }

  constexpr Range::Range (UNSAFE, ID id, ID end) noexcept
    : _begin {id}
    , _end {end}
  {}
  constexpr Range::Range (ID id, ID end)
    : _begin {id}
    , _end {end}
  {
    if (! (_begin < _end))
    {
      throw Error::BeginMustBeSmallerThanEnd {_begin, _end};
    }
  }

  constexpr auto Range::is_extended_by (Range const& rhs) const noexcept -> bool
  {
    return _end == begin (rhs);
  }

  constexpr auto Range::extend_by (Range const& rhs) noexcept -> Range&
  {
    _end = end (rhs);

    return *this;
  }

  constexpr auto UNSAFE_make_range (ID begin, ID end) noexcept -> Range
  {
    return Range {Range::UNSAFE{}, begin, end};
  }

  constexpr auto make_range (ID begin, ID end) -> Range
  {
    return Range {begin, end};
  }

  constexpr auto make_range (ID id, Count count) -> Range
  {
    return make_range (id, id + count);
  }

  constexpr auto begin (Range range) noexcept -> ID
  {
    return range._begin;
  }

  constexpr auto end (Range range) noexcept -> ID
  {
    return range._end;
  }

  constexpr auto size (Range range) noexcept -> Count
  {
    return end (range) - begin (range);
  }
}
