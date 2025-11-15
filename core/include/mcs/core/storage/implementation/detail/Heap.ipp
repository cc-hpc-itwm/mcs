// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <exception>
#include <fmt/ranges.h>
#include <iterator>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <tuple>

namespace mcs::core::storage::implementation
{
  constexpr auto Heap::Error::BadAlloc::requested
    (
    ) const noexcept -> memory::Size
  {
    return _requested;
  }

  constexpr auto Heap::Error::BadAlloc::used
    (
    ) const noexcept -> memory::Size
  {
    return _used;
  }

  constexpr auto Heap::Error::BadAlloc::max
    (
    ) const noexcept -> MaxSize
  {
    return _max;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto Heap::Error::ChunkDescription::parameter
    (
    ) const noexcept -> Parameter::Chunk::Description const&
  {
    return _parameter;
  }

  constexpr auto Heap::Error::ChunkDescription::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }

  constexpr auto Heap::Error::ChunkDescription::memory_range
    (
    ) const noexcept -> core::memory::Range
  {
    return _memory_range;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    Heap::Chunk::Description<Access>::State::State
      ( Description<Access> const& description
      )
        : _bytes
          { memory::select
            ( typename Access::template Span<std::byte>
                { util::cast<typename Access::template Pointer<std::byte>>
                   (description._begin)
                , description._size
                }
            , description._range
            )
          }
  {}
  template<chunk::is_access Access>
    auto Heap::Chunk::Description<Access>::State::bytes
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return _bytes;
  }

  template<chunk::is_access Access>
    auto Heap::chunk_description
      ( Parameter::Chunk::Description parameter
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  try
  {
    auto const buffer {_buffer_by_id.find (segment_id)};
    if (buffer == std::end (_buffer_by_id))
    {
      throw Error::ChunkDescription::UnknownSegmentID{};
    }
    auto const data
      { buffer->second
      . template data<typename Access::template ValueType<std::byte>>()
      };

    return Chunk::Description<Access>
      { util::cast<std::uintmax_t> (data.data())
      , data.size()
      , memory_range
      };
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::ChunkDescription {parameter, segment_id, memory_range}
      );
  }

  template<chunk::is_access Access>
    constexpr Heap::Chunk::Description<Access>::Description
      ( std::uintmax_t begin
      , std::size_t size
      , memory::Range range
      ) noexcept
        : _begin {begin}
        , _size {size}
        , _range {range}
  {}
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "mcs::core::storage::implementation::Heap"
  , mcs::core::storage::implementation::Heap::Tag
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Heap "
  , mcs::core::storage::implementation::Heap::Parameter::Create
  , max_size
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::Size::Max"
  , mcs::core::storage::implementation::Heap::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::Size::Used"
  , mcs::core::storage::implementation::Heap::Parameter::Size::Used
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::Segment::MLOCKed"
  , mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "Heap::Segment::Create "
  , mcs::core::storage::implementation::Heap::Parameter::Segment::Create
  , mlocked
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::Segment::Remove"
  , mcs::core::storage::implementation::Heap::Parameter::Segment::Remove
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::Chunk::Description"
  , mcs::core::storage::implementation::Heap::Parameter::Chunk::Description
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::File::Read"
  , mcs::core::storage::implementation::Heap::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "Heap::File::Write"
  , mcs::core::storage::implementation::Heap::Parameter::File::Write
  );

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , description
      , core::storage::implementation::Heap::Chunk::Description<Access>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, _begin);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, _size);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, _range);

    return oa;
  }

  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::implementation::Heap::Chunk::Description<Access>
      )
  {
    using Description
      = core::storage::implementation::Heap::Chunk::Description<Access>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, _begin, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, _size, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, _range, Description);

    return Description {_begin, _size, _range};
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::Heap::Chunk::Description<Access>
      )
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( description
      , context
      , mcs::core::storage::implementation::Heap::Chunk::Description<Access>
      )
  {
    return fmt::format_to
      ( context.out()
      , "Heap::Chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( description._begin
        , description._size
        , description._range
        )
      );
  }
}
