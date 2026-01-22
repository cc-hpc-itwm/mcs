// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::fuse::content::storage
{
  constexpr Part::Part
    ( core::memory::Range range
    , core::storage::segment::ID segment_id
    ) noexcept
      : _range {range}
      , _segment_id {segment_id}
  {}

  constexpr auto Part::range
    (
    ) const noexcept -> core::memory::Range
  {
    return _range;
  }

  constexpr auto Part::segment_id
    (
    ) const noexcept -> core::storage::segment::ID
  {
    return _segment_id;
  }

  constexpr auto Part::crop (core::memory::Offset end) -> void
  {
    _range = core::memory::make_range (core::memory::begin (_range), end);
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::fuse::content::storage::Part>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::fuse::content::storage::Part>::format
      ( mcs::fuse::content::storage::Part const& part
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to
      ( context.out()
      , "({}, {})"
      , part.segment_id()
      , part.range()
      );
  }
}
