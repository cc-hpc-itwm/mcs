// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <linux/falloc.h>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/realloc.hpp>
#include <mcs/util/syscall/sysconf.hpp>
#include <utility>

namespace mcs::fuse::content::implementation
{
  constexpr auto Realloc::State::fuse_init
    ( ::fuse_conn_info*
    ) const noexcept -> void
  {
    return;
  }
  constexpr auto Realloc::State::fuse_destroy
    (
    ) const noexcept -> void
  {
    return;
  }

  auto Realloc::State::f_bsize
    ( state::reading auto const&
    ) const -> unsigned long
  {
    return util::cast<unsigned long> (util::syscall::sysconf (_SC_PAGE_SIZE));
  }
  auto Realloc::State::f_frsize
    ( state::reading auto const& access_token
    ) const -> unsigned long
  {
    return f_bsize (access_token);
  }
  auto Realloc::State::f_blocks
    ( state::reading auto const&
    ) const -> fsblkcnt_t
  {
    return util::cast<fsblkcnt_t> (util::syscall::sysconf (_SC_PHYS_PAGES));
  }
  auto Realloc::State::f_bfree
    ( state::reading auto const&
    ) const -> fsblkcnt_t
  {
    return util::cast<fsblkcnt_t> (util::syscall::sysconf (_SC_AVPHYS_PAGES));
  }
  auto Realloc::State::f_bavail
    ( state::reading auto const& access_token
    ) const -> fsblkcnt_t
  {
    return f_bfree (access_token);
  }
}

namespace mcs::fuse::content::implementation
{
  Realloc::Capacity::Capacity
    ( util::not_null<state::inode::Common> inode
    , state::writing auto const& write_access
    )
      : _inode {inode}
  {
    set (write_access, 0UL, XATTR_CREATE);
  }

  auto Realloc::Capacity::set
    ( state::writing auto const& write_access
    , size_t size
    ) -> void
  {
    set (write_access, size, XATTR_REPLACE);
  }
  auto Realloc::Capacity::set
    ( state::writing auto const& write_access
    , size_t size
    , int flags
    ) -> void
  {
    _inode->setxattr (write_access, _xattr_key, _value = size, flags);
  }
}

namespace mcs::fuse::content::implementation
{
  Realloc::Realloc
    ( util::not_null<State>
    , util::not_null<state::inode::Common> inode
    , state::writing auto const& write_access
    , ::fuse_ino_t
    )
      : _capacity {inode, write_access}
  {}

  Realloc::~Realloc()
  {
    std::free (_data);
  }

  auto Realloc::resize
    ( state::writing auto const& write_access
    , size_t size
    ) -> content::result::Resize
  {
    _size = size;

    if (_size == 0UL)
    {
      std::free (_data);

      _data = nullptr;

      _capacity.set (write_access, 0UL);
    }
    else if (_capacity < _size)
    {
      auto const new_capacity {std::max (_size, grow (_capacity))};

      _data = mcs::util::cast<std::byte*>
        ( util::syscall::realloc (_data, new_capacity)
        );

      std::ranges::fill
        ( std::span {_data + _capacity, _data + new_capacity}
        , std::byte {0}
        );

      _capacity.set (write_access, new_capacity);
    }
    else if (grow (_size) < _capacity)
    {
      auto const new_capacity
        { std::invoke
          ( [&]() noexcept
            {
              auto _new_capacity {shrink (_capacity)};

              while (grow (_size) < _new_capacity)
              {
                _new_capacity = shrink (_new_capacity);
              }

              return _new_capacity;
            }
          )
        };

      _data = mcs::util::cast<std::byte*>
        ( util::syscall::realloc (_data, new_capacity)
        );

      _capacity.set (write_access, new_capacity);
    }

    return content::result::resize::Success {size};
  }

  auto Realloc::write
    ( state::writing auto const& write_access
    , content::Data const& data
    , off_t offset
    ) -> content::result::Write
  try
  {
    auto new_size {std::optional<size_t>{}};

    if ( auto const size {mcs::util::cast<size_t> (offset) + data.size()}
       ; _size < size
       )
    {
      _size = size;

      if (size > _capacity)
      {
        auto const new_capacity {std::max (_size, grow (_capacity))};

        _data = mcs::util::cast<std::byte*>
          ( util::syscall::realloc (_data, new_capacity)
          );

        _capacity.set (write_access, new_capacity);
      }

      new_size.emplace (_size);
    }

    data.for_each_chunk
      ( [destination {std::next (_data, offset)}]
          ( auto chunk
          ) mutable
        {
          std::ranges::copy (chunk, destination);
          std::advance (destination, chunk.size());
        }
      );

    return content::result::write::Success {new_size};
  }
  catch (std::system_error const& error)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Realloc::write: std::system_error: {}\n"
      , std::current_exception()
      );

    return content::result::write::Error {error.code().value()};
  }
  catch (std::length_error const&)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Realloc::write: std::length_error: {}\n"
      , std::current_exception()
      );

    return content::result::write::Error {ENOMEM};
  }
  catch (std::bad_alloc const&)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Realloc::write: std::bad_alloc\n"
      );

    return content::result::write::Error {ENOMEM};
  }

  constexpr auto Realloc::size
    ( state::reading auto const&
    ) const noexcept -> size_t
  {
    return _size;
  }

  auto Realloc::data
    ( state::reading auto const&
    , size_t size
    , off_t offset
    ) const -> Data
  {
    if (std::cmp_less (offset, _size))
    {
      return content::Data
        { std::span<std::byte const>
          { _data + offset
          , std::min (size, _size - util::cast<size_t> (offset))
          }
        };
     }

    return content::Data{};
  }

  auto Realloc::fallocate
    ( state::writing auto const& write_access
    , int mode
    , off_t offset
    , off_t length
    ) -> int
  {
    if (mode & FALLOC_FL_PUNCH_HOLE)
    {
      return EOPNOTSUPP;
    }
    if (mode & FALLOC_FL_NO_HIDE_STALE)
    {
      return EOPNOTSUPP;
    }
    if (mode & FALLOC_FL_COLLAPSE_RANGE)
    {
      return EOPNOTSUPP;
    }
    if (mode & FALLOC_FL_INSERT_RANGE)
    {
      return EOPNOTSUPP;
    }
    if (mode & FALLOC_FL_UNSHARE_RANGE)
    {
      return EOPNOTSUPP;
    }

    auto const size {this->size (write_access)};
    auto const min_size {util::cast<size_t> (offset + length)};

    if (mode & FALLOC_FL_KEEP_SIZE)
    {
      if (std::cmp_less (size, min_size))
      {
        return EOPNOTSUPP;
      }
    }

    resize (write_access, min_size);

    std::fill
      ( _data + offset
      , _data + std::min (size, min_size)
      , std::byte {0}
      );

    return 0;
  }

  auto Realloc::cleanup
    ( state::writing auto const&
    ) -> void
  {
    return;
  }
}

namespace mcs::fuse::content::implementation
{
  constexpr auto Realloc::grow
    ( std::size_t n
    ) noexcept -> std::size_t
  {
    // n * 3/2
    return (n * 3UL + 1UL) >> 1UL;
  }
  constexpr auto Realloc::shrink
    ( std::size_t n
    ) noexcept -> std::size_t
  {
    // n * 2/3
    return ((n + 1UL) >> 1UL) / 3UL;
  }
}
