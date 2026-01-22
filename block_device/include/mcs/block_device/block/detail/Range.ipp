// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <tuple>

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::block_device::block::Range>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::block_device::block::Range>::format
      ( mcs::block_device::block::Range const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::block_device::block::Range "
      , std::make_tuple (value._begin, value._end)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::block_device::block::Range>::read
      ( State<Char>& state
      ) -> mcs::block_device::block::Range
  {
    prefix (state, "mcs::block_device::block::Range ");

    using Range = mcs::block_device::block::Range;
    return std::make_from_tuple<Range>
      ( parse< std::tuple
               < decltype (Range::_begin)
               , decltype (Range::_end)
               >
             > (state)
      );
  }
}

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
