// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <iterator>
#include <mcs/nonstd/scope.hpp>
#include <mcs/util/FMT/STD/optional.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/ftruncate.hpp>
#include <mcs/util/syscall/mlock.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munlock.hpp>
#include <mcs/util/syscall/shm_open.hpp>
#include <mcs/util/syscall/shm_unlink.hpp>
#include <mcs/util/tuplish/define.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mcs::core::storage::implementation
{
  constexpr auto SHMEM::Error::BadAlloc::requested
    (
    ) const noexcept -> memory::Size
  {
    return _requested;
  }

  constexpr auto SHMEM::Error::BadAlloc::used
    (
    ) const noexcept -> memory::Size
  {
    return _used;
  }

  constexpr auto SHMEM::Error::BadAlloc::max
    (
    ) const noexcept -> MaxSize
  {
    return _max;
  }
}

namespace mcs::core::storage::implementation
{
  constexpr auto SHMEM::Error::ChunkDescription::parameter
    (
    ) const noexcept -> Parameter::Chunk::Description const&
  {
    return _parameter;
  }

  constexpr auto SHMEM::Error::ChunkDescription::segment_id
    (
    ) const noexcept -> segment::ID
  {
    return _segment_id;
  }

  constexpr auto SHMEM::Error::ChunkDescription::memory_range
    (
    ) const noexcept -> core::memory::Range
  {
    return _memory_range;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    SHMEM::Chunk::Description<Access>::State::State
      ( Description<Access> const& description
      )
        : _cache
          { new CacheImpl<Access>
            { typename CacheImpl<Access>::Open{}
            , description.prefix
            , description.segment_id
            , description.size
            }
          , typename CacheImpl<Access>::Deleter{}
          }
        , _bytes {memory::select (_cache->data(), description.range)}
  {}
  template<chunk::is_access Access>
    auto SHMEM::Chunk::Description<Access>::State::bytes
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return _bytes;
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    auto SHMEM::chunk_description
      ( Parameter::Chunk::Description parameter
      , segment::ID segment_id
      , memory::Range memory_range
      ) const -> Chunk::Description<Access>
  try
  {
    auto const cache {_caches_by_id.find (segment_id)};
    if (cache == std::end (_caches_by_id))
    {
      throw Error::ChunkDescription::UnknownSegmentID{};
    }

    return Chunk::Description<Access>
      { cache->second->_prefix
      , cache->second->_segment_id
      , memory::make_size (cache->second->_buffer.size())
      , memory_range
      };
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error::ChunkDescription {parameter, segment_id, memory_range}
      );
  }
}

namespace mcs::core::storage::implementation
{
  template<chunk::is_access Access>
    SHMEM::CacheImpl<Access>::CacheImpl
      ( Create
      , Prefix prefix
      , segment::ID segment_id
      , memory::Size size
      , Parameter::Segment::AccessMode mode
      , std::optional<Parameter::Segment::MLOCKed> mlocked
      )
        : _prefix {prefix}
        , _segment_id {segment_id}
        , _buffer { shm_create ( name()
                               , memory::size_cast<std::size_t> (size)
                               , mode
                               , mlocked
                               )
                  }
  {}

  template<chunk::is_access Access>
    SHMEM::CacheImpl<Access>::CacheImpl
      ( Open
      , Prefix prefix
      , segment::ID segment_id
      , memory::Size size
      )
        : _prefix {prefix}
        , _segment_id {segment_id}
        , _buffer { shm_open ( name()
                             , memory::size_cast<std::size_t> (size)
                             )
                  }
  {}

  template<chunk::is_access Access>
    auto SHMEM::CacheImpl<Access>::Deleter::operator()
      ( CacheImpl* cache
      ) const noexcept -> void
  {
    util::execute_and_die_on_exception
      ( "SHMEM::Deleter"
      , [&]
        {
         if (cache->_unlink.has_value())
         {
           util::syscall::shm_unlink (cache->_unlink->c_str());
         }
         std::default_delete<CacheImpl>{} (cache);
        }
      );
  }

  template<chunk::is_access Access>
    auto SHMEM::CacheImpl<Access>::name() const -> Name
  {
    return fmt::format ("/{}.{}", _prefix.value, _segment_id);
  }

  template<chunk::is_access Access>
    auto SHMEM::CacheImpl<Access>::shm_create
      ( Name name
      , std::size_t size
      , Parameter::Segment::AccessMode mode
      , std::optional<Parameter::Segment::MLOCKed> mlocked
      ) -> decltype (_buffer)
  {
    static_assert (std::is_same_v<Access, chunk::access::Mutable>);

    auto const fd
      { util::syscall::shm_open
          (name.c_str(), O_CREAT | O_EXCL | O_RDWR, mode.value)
      };
    auto const close_fd
      { nonstd::make_scope_exit_that_dies_on_exception
        ( "SHMEM::shm_create::close"
        , [fd]
          {
            util::syscall::close (fd);
          }
        )
      };

    auto const unlink_on_fail
      { nonstd::make_scope_fail_that_dies_on_exception
        ( "SHMEM::shm_create::unlink_on_fail"
        , [&]
          {
            util::syscall::shm_unlink (name.c_str());
          }
        )
      };

    _unlink = name;

    util::syscall::ftruncate (fd, util::cast<off_t> (size));

    auto memory
      { typename decltype (_buffer)::Memory
        { util::syscall::mmap_with_length_zero_allowed
          ( nullptr
          , size
          , PROT_READ | PROT_WRITE // always Mutable
          , MAP_SHARED
          , fd
          , off_t {0}
          )
        , Unmap {size, mlocked.has_value()}
        }
      };

    if (mlocked.has_value())
    {
      util::syscall::mlock (memory.get(), size);
    }

    return decltype (_buffer) {size, std::move (memory)};
  }

  template<chunk::is_access Access>
    auto SHMEM::CacheImpl<Access>::shm_open
      ( Name name
      , std::size_t size
      ) -> decltype (_buffer)
  {
    auto const fd
      { util::syscall::shm_open
          ( name.c_str()
          , chunk::select<Access>
            ( chunk::make_value<chunk::access::Const> (O_RDONLY)
            , chunk::make_value<chunk::access::Mutable> (O_RDWR)
            )
          , 0 // mode is ignored if object already exists
          )
      };
    auto const close_fd
      { nonstd::make_scope_exit_that_dies_on_exception
        ( "SHMEM::shm_open::close_fd"
        , [fd]
          {
            util::syscall::close (fd);
          }
        )
      };

    auto memory
      { typename decltype (_buffer)::Memory
        { util::syscall::mmap_with_length_zero_allowed
          ( nullptr
          , size
          , chunk::select<Access>
            ( chunk::make_value<chunk::access::Const> (PROT_READ)
            , chunk::make_value<chunk::access::Mutable> (PROT_READ | PROT_WRITE)
            )
          , MAP_SHARED
          , fd
          , off_t {0}
          )
        , Unmap {size}
        }
      };

    return decltype (_buffer) {size, std::move (memory)};
  }

  template<chunk::is_access Access>
    auto SHMEM::CacheImpl<Access>::data
      (
      ) const -> typename Access::template Span<std::byte>
  {
    return _buffer.template data<std::byte>();
  }
}

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "mcs::core::storage::implementation::SHMEM"
  , mcs::core::storage::implementation::SHMEM::Tag
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1_SIMPLE
  ( "Prefix "
  , mcs::core::storage::implementation::SHMEM::Prefix
  , value
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ2
  ( "SHMEM "
  , mcs::core::storage::implementation::SHMEM::Parameter::Create
  , prefix
  , max_size
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::Size::Max"
  , mcs::core::storage::implementation::SHMEM::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::Size::Used"
  , mcs::core::storage::implementation::SHMEM::Parameter::Size::Used
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ1
  ( "SHMEM::Segment::AccessMode"
  , mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode
  , value
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::Segment::MLOCKed"
  , mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ2
  ( "SHMEM::Segment::Create "
  , mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create
  , access_mode
  , mlocked
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::Segment::Remove"
  , mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::Chunk::Description"
  , mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description
  );

MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::File::Read"
  , mcs::core::storage::implementation::SHMEM::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DEFINE_FMT_READ0
  ( "SHMEM::File::Write"
  , mcs::core::storage::implementation::SHMEM::Parameter::File::Write
  );

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_OUTPUT
      ( oa
      , description
      , core::storage::implementation::SHMEM::Chunk::Description<Access>
      )
  {
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, prefix);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, segment_id);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, size);
    MCS_SERIALIZATION_SAVE_FIELD (oa, description, range);

    return oa;
  }

  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DEFINE_NONINTRUSIVE_IMPLEMENTATION_INPUT
      ( ia
      , core::storage::implementation::SHMEM::Chunk::Description<Access>
      )
  {
    using Description
      = core::storage::implementation::SHMEM::Chunk::Description<Access>
      ;

    MCS_SERIALIZATION_LOAD_FIELD (ia, prefix, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, segment_id, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, size, Description);
    MCS_SERIALIZATION_LOAD_FIELD (ia, range, Description);

    return Description {prefix, segment_id, size, range};
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_PARSE
      ( context
      , mcs::core::storage::implementation::SHMEM::Chunk::Description<Access>
      )
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DEFINE_FORMAT
      ( description
      , context
      , mcs::core::storage::implementation::SHMEM::Chunk::Description<Access>
      )
  {
    return fmt::format_to
      ( context.out()
      , "SHMEM::Chunk::Description<{}> {}"
      , Access{}
      , std::make_tuple
        ( description.prefix
        , description.segment_id
        , description.size
        , description.range
        )
      );
  }
}
