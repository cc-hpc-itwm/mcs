// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <iterator>
#include <mcs/nonstd/scope.hpp>
#include <mcs/util/FMT/STD/optional.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/execute_and_die_on_exception.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/ftruncate.hpp>
#include <mcs/util/syscall/mlock.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munlock.hpp>
#include <mcs/util/syscall/shm_open.hpp>
#include <mcs/util/syscall/shm_unlink.hpp>
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

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Tag>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Tag>::format
      ( mcs::core::storage::implementation::SHMEM::Tag const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "mcs::core::storage::implementation::SHMEM"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Tag>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Tag
  {
    prefix (state, "mcs::core::storage::implementation::SHMEM");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Tag>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Prefix>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Prefix>::format
      ( mcs::core::storage::implementation::SHMEM::Prefix const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to (ctx.out(), "{}{}", "Prefix ", value.value);
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Prefix>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Prefix
  {
    prefix (state, "Prefix ");

    using Prefix = mcs::core::storage::implementation::SHMEM::Prefix;

    return Prefix {parse<decltype (Prefix::value)> (state)};
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Create>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM "
      , std::make_tuple (value.prefix, value.max_size)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Create
  {
    prefix (state, "SHMEM ");

    using Create = mcs::core::storage::implementation::SHMEM::Parameter::Create;
    return std::make_from_tuple<Create>
      ( parse< std::tuple
               < decltype (Create::prefix)
               , decltype (Create::max_size)
               >
             > (state)
      );
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Size::Max const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Size::Max"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Size::Max
  {
    prefix (state, "SHMEM::Size::Max");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Size::Used const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Size::Used"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Size::Used
  {
    prefix (state, "SHMEM::Size::Used");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Segment::AccessMode"
      , std::make_tuple (value.value)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode
  {
    prefix (state, "SHMEM::Segment::AccessMode");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>
      (parse<std::tuple<decltype (mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode::value)>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Segment::MLOCKed"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed
  {
    prefix (state, "SHMEM::Segment::MLOCKed");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create const& value
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Segment::Create "
      , std::make_tuple (value.access_mode, value.mlocked)
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create
  {
    prefix (state, "SHMEM::Segment::Create ");

    using Create = mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create;
    return std::make_from_tuple<Create>
      ( parse< std::tuple
               < decltype (Create::access_mode)
               , decltype (Create::mlocked)
               >
             > (state)
      );
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Segment::Remove"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove
  {
    prefix (state, "SHMEM::Segment::Remove");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::Chunk::Description"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description
  {
    prefix (state, "SHMEM::Chunk::Description");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::File::Read const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::File::Read"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::File::Read
  {
    prefix (state, "SHMEM::File::Read");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>
      (parse<std::tuple<>> (state));
  }
}

namespace fmt
{
  template<typename ParseContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>::parse (ParseContext& ctx)
  {
    return ctx.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>::format
      ( mcs::core::storage::implementation::SHMEM::Parameter::File::Write const&
      , FormatContext& ctx
      ) const -> decltype (ctx.out())
  {
    return fmt::format_to
      ( ctx.out()
      , "{}{}"
      , "SHMEM::File::Write"
      , std::make_tuple()
      );
  }
}

namespace mcs::util::read
{
  template<typename Char>
    auto Read<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>::read
      ( State<Char>& state
      ) -> mcs::core::storage::implementation::SHMEM::Parameter::File::Write
  {
    prefix (state, "SHMEM::File::Write");

    return std::make_from_tuple<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>
      (parse<std::tuple<>> (state));
  }
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::SHMEM::Chunk::Description<Access>>::output
      ( OArchive& oa
      , core::storage::implementation::SHMEM::Chunk::Description<Access> const& description
      ) -> OArchive&
  {
    save (oa, description.prefix);
    save (oa, description.segment_id);
    save (oa, description.size);
    save (oa, description.range);

    return oa;
  }

  template<core::chunk::is_access Access>
    auto Implementation<core::storage::implementation::SHMEM::Chunk::Description<Access>>::input
      ( IArchive& ia
      ) -> core::storage::implementation::SHMEM::Chunk::Description<Access>
  {
    using Description
      = core::storage::implementation::SHMEM::Chunk::Description<Access>
      ;

    auto prefix {load<decltype (Description::prefix)> (ia)};
    auto segment_id {load<decltype (Description::segment_id)> (ia)};
    auto size {load<decltype (Description::size)> (ia)};
    auto range {load<decltype (Description::range)> (ia)};

    return Description {prefix, segment_id, size, range};
  }
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    template<typename ParseContext>
      constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Chunk::Description<Access>>::parse (ParseContext& context)
  {
    return context.begin();
  }

  template<mcs::core::chunk::is_access Access>
    template<typename FormatContext>
      constexpr auto formatter<mcs::core::storage::implementation::SHMEM::Chunk::Description<Access>>::format
        ( mcs::core::storage::implementation::SHMEM::Chunk::Description<Access> const& description
        , FormatContext& context
        ) const -> decltype (context.out())
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
