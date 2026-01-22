// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <limits>
#include <mcs/Error.hpp>
#include <mcs/fuse/content/implementation/storage/ioctl/command/Distribution.hpp>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/overloaded.hpp>
#include <mcs/util/syscall/sysconf.hpp>
#include <utility>

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    Storage<StorageImplementation>::State::State
      ( typename Parameter::Create parameter_create
      , typename Parameter::Size::Max parameter_size_max
      , typename Parameter::Size::Used parameter_size_used
      , typename Parameter::Segment::Create parameter_segment_create
      , typename Parameter::Segment::Remove parameter_segment_remove
      , typename Parameter::Chunk::Description parameter_chunk_description
      )
    : _storage {parameter_create}
    , _parameter_size_max {parameter_size_max}
    , _parameter_size_used {parameter_size_used}
    , _parameter_segment_create {parameter_segment_create}
    , _parameter_segment_remove {parameter_segment_remove}
    , _parameter_chunk_description {parameter_chunk_description}
  {}

  template<core::storage::is_implementation StorageImplementation>
    constexpr auto Storage<StorageImplementation>::State::fuse_init
      ( ::fuse_conn_info*
      ) const noexcept -> void
  {
    return;
  }
  template<core::storage::is_implementation StorageImplementation>
    constexpr auto Storage<StorageImplementation>::State::fuse_destroy
      (
      ) const noexcept -> void
  {
    return;
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::f_bsize
      ( state::reading auto const&
      ) const -> unsigned long
  {
    // \todo parameter, depends on the concrete storage
    return util::cast<unsigned long> (util::syscall::sysconf (_SC_PAGE_SIZE));
  }
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::f_frsize
      ( state::reading auto const&
      ) const -> unsigned long
  {
    return 1UL;
  }
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::f_blocks
      ( state::reading auto const&
      ) const -> fsblkcnt_t
  {
    auto const size_max {_storage.size_max (_parameter_size_max)};

    if (size_max.is_unlimited())
    {
      return std::numeric_limits<fsblkcnt_t>::max();
    }

    return size_cast<fsblkcnt_t> (size_max.limit());
  }
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::f_bfree
      ( state::reading auto const& access_token
      ) const -> fsblkcnt_t
  {
    auto const size_used {_storage.size_used (_parameter_size_used)};

    return f_blocks (access_token) - size_cast<fsblkcnt_t> (size_used);
  }
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::f_bavail
      ( state::reading auto const& access_token
      ) const -> fsblkcnt_t
  {
    return f_bfree (access_token);
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::segment_remove
      ( state::writing auto const&
      , core::storage::segment::ID segment_id
      ) -> core::memory::Size
  {
    return _storage.segment_remove (_parameter_segment_remove, segment_id);
  }
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::State::segment_create
      ( state::writing auto const&
      , core::memory::Size size
      ) -> core::storage::segment::ID
  {
    return _storage.segment_create (_parameter_segment_create, size);
  }
  template<core::storage::is_implementation StorageImplementation>
    template<core::chunk::is_access Access>
      auto Storage<StorageImplementation>::State::chunk
        ( state::reading auto const&
        , core::storage::segment::ID segment_id
        , core::memory::Range range
        ) const -> core::Chunk<Access, StorageImplementation>
  {
    return core::Chunk<Access, StorageImplementation>
      { _storage.template chunk_description<Access>
        ( _parameter_chunk_description
        , segment_id
        , range
        )
      };
  }
}

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    constexpr Storage<StorageImplementation>::Data::operator
      content::Data const&
        (
        ) const noexcept
  {
    return _data;
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::Data::emplace
      ( CoreChunk chunk
      ) -> void
  {
    _data.emplace (_chunks.emplace_back (std::move (chunk)).data());
  }
}

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    Storage<StorageImplementation>::Storage
      ( util::not_null<State> state
      , util::not_null<state::inode::Common> inode
      , state::writing auto const& write_access
      , ::fuse_ino_t
      )
        : _state {state}
        , _inode {inode}
  {
    xattr_set_capacity (write_access, XATTR_CREATE);
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::cleanup
      ( state::writing auto const& write_access
      ) -> void
  {
    free_all_parts (write_access);
  }
}

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    constexpr auto Storage<StorageImplementation>::size
      ( state::reading auto const&
      ) const noexcept -> size_t
  {
    return _size;
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::data
      ( state::reading auto const& access_token
      , size_t size
      , off_t offset
      ) const -> Data
  {
    auto data {Data{}};

    for_each_chunk<core::chunk::access::Const>
      ( access_token
      , core::memory::make_range
        ( core::memory::make_offset (offset)
        , core::memory::make_size (size)
        )
      , [&] (auto chunk)
        {
          data.emplace (std::move (chunk));
        }
      );

    return data;
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::resize
      ( state::writing auto const& write_access
      , size_t new_size
      ) -> content::result::Resize
  try
  {
    // Finally update the xattr.
    //
    auto const resized
      { [&]
        {
          _size = new_size;
          xattr_set_capacity (write_access, XATTR_REPLACE);

          return content::result::resize::Success {new_size};
        }
      };

    auto const overwrite_with_zero
      { [&] (auto part, auto range)
        {
          auto const chunk
            { make_chunk<core::chunk::access::Mutable>
              ( write_access
              , part
              , range
              )
            };

          std::ranges::fill (chunk.data(), std::byte {0});
        }
      };

    auto const make_new_range
      { [&] (auto range)
        {
          auto const part
            { _parts.emplace_back
              ( range
              , _state->segment_create
                ( write_access
                , core::memory::size (range)
                )
              )
            };

          // \todo retrieve the information from the storage about
          // whether or not the new segment is initialized to zero,
          // e.g. SHMEM and Files do initialize to zero while Heap
          // does not.
          //
          // overwrite_with_zero (part, range);
          std::ignore = part;

          _capacity = offset_cast<std::size_t> (core::memory::end (range));
        }
      };

    auto const grow_last_part_to
      { [&] (auto new_end)
        {
          if (_size < offset_cast<std::size_t> (new_end))
          {
            auto const old_end {core::memory::end (_parts.back().range())};

            _parts.back().crop (new_end);

            overwrite_with_zero
              ( _parts.back()
              , core::memory::make_range (old_end, new_end)
              );
          }
        }
      };

    // Situations with respect to new_size in relation to:
    //
    // _size     -- the current size
    // _capacity -- the current capacity
    //
    // It always holds:
    //
    // - _parts.empty() <=> _capacity == 0
    // - 0 <= _size <= _capacity
    // - If _size < _capacity, then _size is inside the last part.
    //
    //   0                                   _size       _capacity
    //                                       v           v
    //   [-----)[---------)[-------------)[--------------)
    //   :  :              :                 :    :      :     :
    //   ^  :              :                 :    :      :     :
    // a new_size          :                 :    :      :     :
    //      :              :                 :    :      :     :
    //      ^..            :                 :    :      :     :
    // b    new_size       :                 :    :      :     :
    //                     ^                 :    :      :     :
    // c                   new_size          :    :      :     :
    //                                       ^    :      :     :
    // d                                     new_size    :     :
    //                                            ^      :     :
    // e                                     0000 new_size     :
    //                                                   ^     :
    // f                                     00000000000 new_size
    //                                                   :     ^
    // g                                     000000000000:0000 new_size
    //
    // a - new size is zero
    // b - new size < _size, in the middle of a part
    // c - new size < _size, the begin of a part
    // d - new size == _size (can this happen?)
    // e - _size < new size, new size < _capacity
    // f - _size < new size, new size == _capacity
    // g - _size < new size, new size > _capacity
    //
    // Data marked 0 must be overwritten with zero by the definition
    // of "read after resize".
    //
    // \todo data marked .. in case b is not freed but unreadable
    // data: Should it be overwritten?
    //
    // \todo data freed (in a, b, c): Should it be overwritten before freed?
    //

    if (std::cmp_equal (new_size, 0)) // a
    {
      free_all_parts (write_access);

      return resized();
    }

    if (std::cmp_equal (_capacity, 0)) // g with capacity == 0
    {
      make_new_range
        ( core::memory::make_range
          ( core::memory::make_offset (0)
          , core::memory::make_offset (new_size)
          )
        );

      return resized();
    }

    if (_parts.empty())
    {
      throw mcs::Error {"INCONSISTENT: _capacity > 0, _parts.empty()"};
    }

    if (std::cmp_less (_capacity, new_size)) // g with capacity > 0
    {
      grow_last_part_to (core::memory::make_offset (_capacity));

      make_new_range
        ( core::memory::make_range
          ( core::memory::make_offset (_capacity)
          , core::memory::make_offset (new_size)
          )
        );

      return resized();
    }

    if (std::cmp_equal (_capacity, new_size)) // f
    {
      grow_last_part_to (core::memory::make_offset (new_size));

      return resized();
    }

    if (std::cmp_less (_size, new_size)) // e
    {
      grow_last_part_to (core::memory::make_offset (new_size));

      return resized();
    }

    if (std::cmp_equal (_size, new_size)) // d
    {
      return content::result::resize::Success {new_size};
    }

    auto const new_end {core::memory::make_offset (new_size)};
    auto const part {lower_bound (new_end)};

    if (part == _parts.end())
    {
      throw mcs::Error {"INCONSISTENT: offset < _size has no part"};
    }

    if (core::memory::begin (part->range()) == new_end) // c
    {
      free_range_of_parts (write_access, part, std::end (_parts));

      return resized();
    }

    // b
    free_range_of_parts (write_access, std::next (part), std::end (_parts));
    part->crop (new_end);

    return resized();
  }
  catch (std::system_error const& error)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Storage::resize: std::system_error: {}\n"
      , std::current_exception()
      );

    return content::result::resize::Error {error.code().value()};
  }
  catch (typename StorageImplementation::Error::BadAlloc const& bad_alloc)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Storage::resize: {}\n"
      , bad_alloc.what()
      );

    return content::result::resize::Error {ENOMEM};
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::write
      ( state::writing auto const& write_access
      , content::Data const& data
      , off_t offset
      ) -> content::result::Write
  try
  {
    if (data.size() == 0)
    {
      return content::result::write::Success{};
    }

    using core::memory::begin;
    using core::memory::end;

    auto const range
      { core::memory::make_range
        ( core::memory::make_offset (offset)
        , core::memory::make_size (data.size())
        )
      };

    auto const new_size
      { std::invoke
        ( [&]() noexcept -> std::optional<size_t>
          {
            auto const size_wanted {offset_cast<size_t> (end (range))};
            if (size (write_access) < size_wanted)
            {
              return size_wanted;
            }
            return {};
          }
        )
      };

    if (new_size.has_value())
    {
      resize (write_access, *new_size);
    }

    auto const& data_chunks {data.chunks()};
    auto data_chunk {std::begin (data_chunks)};
    auto source {*data_chunk};

    for_each_chunk<core::chunk::access::Mutable>
      ( write_access
      , range
      , [&] (auto destination_chunk)
        {
          auto destination {destination_chunk.data()};

          while (destination.size() > 0)
          {
            auto const number_of_bytes
              { std::min (source.size(), destination.size())
              };

            std::ranges::copy
              ( source.subspan (0, number_of_bytes)
              , std::begin (destination)
              );

            destination = destination.subspan (number_of_bytes);
            source = source.subspan (number_of_bytes);
            while (source.size() == 0)
            {
              source = *++data_chunk;
            }
          }
        }
      );

    return content::result::write::Success {new_size};
  }
  catch (std::system_error const& error)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Storage::write: std::system_error: {}\n"
      , std::current_exception()
      );

    return content::result::write::Error {error.code().value()};
  }
  catch (std::length_error const&)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Storage::write: std::length_error: {}\n"
      , std::current_exception()
      );

    return content::result::write::Error {ENOMEM};
  }
  catch (std::bad_alloc const&)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Storage::write: std::bad_alloc\n"
      );

    return content::result::write::Error {ENOMEM};
  }
  catch (typename StorageImplementation::Error::BadAlloc const& bad_alloc)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Storage::write: {}\n"
      , bad_alloc.what()
      );

    return content::result::write::Error {ENOMEM};
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::fallocate
      ( state::writing auto const&
      , int
      , off_t
      , off_t
      ) -> int
  {
    return EOPNOTSUPP;
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::ioctl
      ( state::reading auto const&
      , ::fuse_req_t request
      , unsigned int command
      , void *arg
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      ) const -> void
  {
    std::ignore = arg;
    std::ignore = out_buf_size;

    switch (command)
    {
      break; case storage::ioctl::command::Distribution::CODE:
      {
        using Distribution = storage::ioctl::command::Distribution;

        if (in_buf_size != sizeof (Distribution::Buffer))
        {
          throw mcs::Error
            { fmt::format ( "in_buf has wrong size: {} != {}"
                          , in_buf_size
                          , sizeof (Distribution::Buffer)
                          )
            };
        }

        auto in_buffer {util::cast<Distribution::Buffer const*> (in_buf)};

        auto const out_buffer {Distribution::Buffer {in_buffer, _parts}};

        return reply::ioctl
          ( request
          , 0
          , std::addressof (out_buffer)
          , sizeof (out_buffer)
          );
      }
      break; default: return reply::error (request, ENOSYS);
    }
  }
}

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    template<core::chunk::is_access Access>
      auto Storage<StorageImplementation>::make_chunk
        ( state::reading auto const& access_token
        , storage::Part const& part
        , core::memory::Range range
        ) const -> core::Chunk<Access, StorageImplementation>
  {
    using core::memory::begin;
    using core::memory::end;

    auto const assert_range_is_inside_of_part
      { [&]
        {
          if (begin (range) < begin (part.range()))
          {
            throw mcs::Error
              { fmt::format
                ( "content::Storage::make_chunk: {} is not inside of part {}"
                , range
                , part.range()
                )
              };
          }

          if (end (part.range()) < end (range))
          {
            throw mcs::Error
              { fmt::format
                ( "content::Storage::make_chunk: {} is not inside of part {}"
                , range
                , part.range()
                )
              };
          }
        }
      };
    std::invoke (assert_range_is_inside_of_part);

    return _state->template chunk<Access>
      ( access_token
      , part.segment_id()
      , core::memory::shift (range, -begin (part.range()))
      );
  }

  template<core::storage::is_implementation StorageImplementation>
    template<core::chunk::is_access Access, typename UseChunk>
      requires (std::invocable< UseChunk
                              , core::Chunk<Access, StorageImplementation>
                              >
               )
      auto Storage<StorageImplementation>::for_each_chunk
        ( state::reading auto const& access_token
        , core::memory::Range range
        , UseChunk use_chunk
        ) const -> void
  {
    using core::memory::begin;
    using core::memory::end;

    for ( auto part {lower_bound (begin (range))}
        ; part != std::end (_parts) && begin (part->range()) < end (range)
        ; ++part
        )
    {
      // in the first iteration that is the postcondition of
      // lower_bound, in later iterations this is guaranteed by the
      // order of the set
      assert (begin (range) < end (part->range()));

      auto const intersection
        { core::memory::make_range
          ( std::max (begin (part->range()), begin (range))
          , std::min (end   (part->range()), end   (range))
          )
        };

      std::invoke
        ( use_chunk
        , make_chunk<Access> (access_token, *part, intersection)
        );
    }
  }
}

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::xattr_set_capacity
      ( state::writing auto const& write_access
      , int flags
      ) -> void
  {
    _inode->setxattr
      ( write_access
      , _xattr_key_capacity
      , _capacity
      , flags
      );
  }
}

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::free_all_parts
      ( state::writing auto const& write_access
      ) -> void
    {
      free_range_of_parts
        ( write_access
        , std::begin (_parts)
        , std::end (_parts)
        );
    }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::free_range_of_parts
      ( state::writing auto const& write_access
      , Parts::iterator begin
      , Parts::iterator end
      ) -> void
    {
      std::for_each
        ( begin, end
        , [&] (auto const& part)
          {
            _capacity -= core::memory::size_cast<size_t>
              ( _state->segment_remove (write_access, part.segment_id())
              );
          }
        );

      _parts.erase (begin, end);
    }
}

namespace mcs::fuse::content::implementation
{
  namespace detail
  {
    template<typename Parts>
      [[nodiscard]] auto lower_bound
        ( Parts& parts
        , core::memory::Offset begin
        )
    {
      return std::ranges::lower_bound
        ( parts
        , begin
        , [&] ( core::memory::Offset part_end
              , core::memory::Offset offset
              ) noexcept
          {
            return ! std::less{} (offset, part_end);
          }
        , [&] (storage::Part const& part) noexcept
          {
            return core::memory::end (part.range());
          }
        );
    }
  }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::lower_bound
      ( core::memory::Offset begin
      ) -> Parts::iterator
    {
      return detail::lower_bound (_parts, begin);
    }

  template<core::storage::is_implementation StorageImplementation>
    auto Storage<StorageImplementation>::lower_bound
      ( core::memory::Offset begin
      ) const -> Parts::const_iterator
    {
      return detail::lower_bound (_parts, begin);
    }
}
