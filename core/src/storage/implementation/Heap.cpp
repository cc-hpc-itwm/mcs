// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/storage/implementation/Heap.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/IArchive.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/load.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/Copy.hpp>
#include <mcs/util/syscall/mlock.hpp>
#include <mcs/util/syscall/munlock.hpp>
#include <mcs/util/touch.hpp>

namespace mcs::core::storage::implementation
{
  Heap::Error::BadAlloc::BadAlloc
    ( memory::Size requested
    , memory::Size used
    , MaxSize max
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::Heap::BadAlloc: requested {}, used {}, max {}"
          , requested
          , used
          , max
          )
        }
      , _requested {requested}
      , _used {used}
      , _max {max}
  {}
  Heap::Error::BadAlloc::~BadAlloc() = default;
}

namespace mcs::core::storage::implementation
{
  Heap::Error::ChunkDescription::ChunkDescription
    ( Parameter::Chunk::Description parameter
    , segment::ID segment_id
    , memory::Range memory_range
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::Heap::ChunkDescription"
            ": segment_id '{}'"
            ", memory_range '{}'"
            ", parameter '{}'"
          , segment_id
          , memory_range
          , parameter
          )
        }
      , _parameter {parameter}
      , _segment_id {segment_id}
      , _memory_range {memory_range}
  {}
  Heap::Error::ChunkDescription::~ChunkDescription() = default;
}

namespace mcs::core::storage::implementation
{
  Heap::Error::ChunkDescription::UnknownSegmentID::UnknownSegmentID() noexcept
    : mcs::Error {"Unknown segment id"}
  {}
  Heap::Error::ChunkDescription::UnknownSegmentID::~UnknownSegmentID
    (
    ) = default
    ;
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Heap::Parameter::Create
  , max_size
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::Heap::Parameter::Segment::Create
  , mlocked
  );

namespace mcs::core::storage::implementation
{
  Heap::Heap (Parameter::Create create) noexcept
    : _max_size {create.max_size}
  {}

  auto Heap::size_max
    ( Parameter::Size::Max
    ) const -> MaxSize
  {
    return _max_size;
  }
  auto Heap::size_used
    ( Parameter::Size::Used
    ) const -> memory::Size
  {
    return _size_used;
  }

  auto Heap::Deleter::operator() (std::byte* ptr) const -> void
  {
    if (_unlock.has_value())
    {
      util::syscall::munlock (ptr, *_unlock);
    }

    std::default_delete<std::byte[]>{} (ptr);
  }

  auto Heap::segment_create
    ( Parameter::Segment::Create create
    , memory::Size size
    ) -> segment::ID
  {
    auto const increment_segment_id
      {nonstd::make_scope_success ([&]() noexcept { ++_next_segment_id; })};
    auto const increment_size_used
      {nonstd::make_scope_success ([&]() noexcept { _size_used += size; })};

    if (_size_used + size > _max_size)
    {
      throw Error::BadAlloc {size, _size_used, _max_size};
    }

    auto const std_size {memory::size_cast<std::size_t> (size)};

    auto memory
      { typename Buffer::Memory
        { new std::byte[std_size]
        , { create.mlocked.has_value()
            ? std::optional<std::size_t> {std_size}
            : std::optional<std::size_t>{}
          }
        }
      };

    if (create.mlocked.has_value())
    {
      util::syscall::mlock (memory.get(), std_size);
    }

    if (! _buffer_by_id.try_emplace
            ( _next_segment_id
            , std_size
            , std::move (memory)
            ).second
       )
    {
      throw mcs::Error
        { fmt::format ("Duplicate segment id {}", _next_segment_id)
        };
    }

    return _next_segment_id;
  }

  auto Heap::segment_remove
    ( Parameter::Segment::Remove
    , segment::ID segment_id
    ) -> memory::Size
  {
    auto& buffer {_buffer_by_id.at (segment_id)};

    auto const size_freed {memory::make_size (buffer.size())};

    _size_used -= size_freed;

    _buffer_by_id.erase (segment_id);

    return size_freed;
  }

  auto Heap::file_read
    ( Parameter::File::Read
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  {
    return memory::make_size
      ( util::Copy{}
        ( util::copy::FileReadLocation {path, make_off_t (begin (range))}
        , memory::select
          ( _buffer_by_id.at (segment_id).data<std::byte>()
          , make_range (offset, size (range))
          ).data()
        , size_cast<std::size_t> (size (range))
        )
      );
  }

  auto Heap::file_write
    ( Parameter::File::Write
    , segment::ID segment_id
    , memory::Offset offset
    , std::filesystem::path path
    , memory::Range range
    ) const -> memory::Size
  {
    return memory::make_size
      ( util::Copy{}
        ( memory::select
          ( _buffer_by_id.at (segment_id).data<std::byte const>()
          , make_range (offset, size (range))
          ).data()
        , util::copy::FileWriteLocation
            { util::touch (path)
            , make_off_t (begin (range))
            }
        , size_cast<std::size_t> (size (range))
        )
      );
  }
}
