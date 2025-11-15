// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/select.hpp>

namespace mcs::core::memory
{
  constexpr Range::Range (Offset begin, Offset end)
    : _begin {begin}
    , _end {end}
  {
    if (_begin > _end)
    {
      throw Error::BeginMustNotBeLargerThanEnd {_begin, _end};
    }
  }

  constexpr auto make_range (Offset begin, Offset end) -> Range
  {
    return Range {begin, end};
  }

  constexpr auto make_range (Offset begin, Size size) -> Range
  {
    return Range {begin, make_offset (begin._value + size._value)};
  }

  constexpr auto begin (Range const& range) -> Offset
  {
    return range._begin;
  }

  constexpr auto end (Range const& range) -> Offset
  {
    return range._end;
  }

  constexpr auto size (Range const& range) -> Size
  {
    return range._end - range._begin;
  }

  constexpr auto shift (Range range, Offset offset) -> Range
  {
    return make_range (offset + begin (range), offset + end (range));
  }
}

namespace mcs::core::memory
{
  template<std::integral I>
    constexpr auto make_range (I begin, I end) -> Range
  {
    return make_range (make_offset (begin), make_offset (end));
  }
}

namespace mcs::core::memory
{
  template<typename T>
    constexpr auto select
      ( std::span<T> data
      , Range const& range
      )
  {
    return util::select (data, begin (range)._value, size (range)._value);
  }
}


namespace mcs::core::memory
{
  constexpr auto Range::Error::BeginMustNotBeLargerThanEnd::begin
    (
    ) const noexcept -> Offset
  {
    return _begin;
  }
  constexpr auto Range::Error::BeginMustNotBeLargerThanEnd::end
    (
    ) const noexcept -> Offset
  {
    return _end;
  }
}

namespace fmt
{
  MCS_UTIL_FMT_DEFINE_PARSE (ctx, mcs::core::memory::Range)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (range, ctx, mcs::core::memory::Range)
  {
    return fmt::format_to (ctx.out(), "[{}..{})", range._begin, range._end);
  }
}

namespace mcs::util::read
{
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, core::memory::Range)
  {
    prefix (state, "[");
    auto begin {parse<core::memory::Offset> (state)};
    prefix (state, "..");
    auto end {parse<core::memory::Offset> (state)};
    prefix (state, ")");

    return core::memory::make_range (begin, end);
  }
}
