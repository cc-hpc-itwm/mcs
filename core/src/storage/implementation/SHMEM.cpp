// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/core/storage/implementation/SHMEM.hpp>
#include <mcs/nonstd/scope.hpp>
#include <mcs/serialization/define.hpp>
#include <mcs/util/Copy.hpp>
#include <mcs/util/syscall/munmap.hpp>
#include <mcs/util/touch.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <utility>

namespace mcs::core::storage::implementation
{
  auto SHMEM::Unmap::operator()
    ( void* ptr
    ) const -> void
  {
    if (_unlock)
    {
      util::syscall::munlock (ptr, _size);
    }

    util::syscall::munmap (ptr, _size);
  }

  SHMEM::Error::BadAlloc::BadAlloc
    ( memory::Size requested
    , memory::Size used
    , MaxSize max
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::SHMEM::BadAlloc: requested {}, used {}, max {}"
          , requested
          , used
          , max
          )
        }
      , _requested {requested}
      , _used {used}
      , _max {max}
  {}
  SHMEM::Error::BadAlloc::~BadAlloc() = default;
}

namespace mcs::core::storage::implementation
{
  SHMEM::Error::ChunkDescription::ChunkDescription
    ( Parameter::Chunk::Description parameter
    , segment::ID segment_id
    , memory::Range memory_range
    ) noexcept
      : mcs::Error
        { fmt::format
          ( "storage::implementation::SHMEM::ChunkDescription"
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
  SHMEM::Error::ChunkDescription::~ChunkDescription() = default;
}

namespace mcs::core::storage::implementation
{
  SHMEM::Error::ChunkDescription::UnknownSegmentID::UnknownSegmentID() noexcept
    : mcs::Error {"Unknown segment id"}
  {}
  SHMEM::Error::ChunkDescription::UnknownSegmentID::~UnknownSegmentID
    (
    ) = default
    ;
}

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::SHMEM::Prefix
  , value
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::core::storage::implementation::SHMEM::Parameter::Create
  , prefix
  , max_size
  );

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION2
  ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create
  , access_mode
  , mlocked
  );

namespace mcs::core::storage::implementation
{
  SHMEM::SHMEM (Parameter::Create create) noexcept
    : _prefix {create.prefix}
    , _max_size {create.max_size}
  {}

  auto SHMEM::size_max
    ( Parameter::Size::Max
    ) const -> MaxSize
  {
    return _max_size;
  }
  auto SHMEM::size_used
    ( Parameter::Size::Used
    ) const -> memory::Size
  {
    return _size_used;
  }

  auto SHMEM::segment_create
    ( Parameter::Segment::Create parameter_create
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

    if (! _caches_by_id.try_emplace
            ( _next_segment_id
            , new CacheImpl<chunk::access::Mutable>
              { typename CacheImpl<chunk::access::Mutable>::Create{}
              , _prefix
              , _next_segment_id
              , size
              , parameter_create.access_mode
              , parameter_create.mlocked
              }
            , typename CacheImpl<chunk::access::Mutable>::Deleter{}
            ).second
       )
    {
      throw mcs::Error
        { fmt::format ("Duplicate segment id {}", _next_segment_id)
        };
    }

    return _next_segment_id;
  }

  auto SHMEM::segment_remove
    ( Parameter::Segment::Remove
    , segment::ID segment_id
    ) -> memory::Size
  {
    auto& cache {_caches_by_id.at (segment_id)};

    auto const size_freed
      { cache->_unlink
      ? memory::make_size (cache->data().size())
      : memory::make_size (0)
      };

    _size_used -= size_freed;

    _caches_by_id.erase (segment_id);

    return size_freed;
  }

  auto SHMEM::file_read
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
         ( _caches_by_id.at (segment_id)->data()
         , make_range (offset, size (range))
         ).data()
        , size_cast<std::size_t> (size (range))
        )
      );
  }

  auto SHMEM::file_write
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
          ( _caches_by_id.at (segment_id)->data()
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

MCS_UTIL_TUPLISH_DEFINE_SERIALIZATION1
  ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode
  , value
  );
