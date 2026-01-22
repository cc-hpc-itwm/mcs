// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/sysconf.hpp>
#include <utility>

namespace mcs::fuse::content::implementation
{
  constexpr auto Vector::State::fuse_init
    ( ::fuse_conn_info*
    ) const noexcept -> void
  {
    return;
  }
  constexpr auto Vector::State::fuse_destroy
    (
    ) const noexcept -> void
  {
    return;
  }

  auto Vector::State::f_bsize
    ( state::reading auto const&
    ) const -> unsigned long
  {
    return util::cast<unsigned long> (util::syscall::sysconf (_SC_PAGE_SIZE));
  }
  auto Vector::State::f_frsize
    ( state::reading auto const& access_token
    ) const -> unsigned long
  {
    return f_bsize (access_token);
  }
  auto Vector::State::f_blocks
    ( state::reading auto const&
    ) const -> fsblkcnt_t
  {
    return util::cast<fsblkcnt_t> (util::syscall::sysconf (_SC_PHYS_PAGES));
  }
  auto Vector::State::f_bfree
    ( state::reading auto const&
    ) const -> fsblkcnt_t
  {
    return util::cast<fsblkcnt_t> (util::syscall::sysconf (_SC_AVPHYS_PAGES));
  }
  auto Vector::State::f_bavail
    ( state::reading auto const& access_token
    ) const -> fsblkcnt_t
  {
    return f_bfree (access_token);
  }
}

namespace mcs::fuse::content::implementation
{
  Vector::Vector
    ( util::not_null<State>
    , util::not_null<state::inode::Common> inode
    , state::writing auto const& write_access
    , ::fuse_ino_t
    )
      : _inode {inode}
  {
    xattr_set_capacity (write_access, XATTR_CREATE);
  }

  constexpr auto Vector::size
    ( state::reading auto const&
    ) const noexcept -> size_t
  {
    return std::size (_data);
  }

  auto Vector::data
    ( state::reading auto const&
    , size_t size
    , off_t offset
    ) const -> Data
  {
    if (std::cmp_less (offset, std::size (_data)))
    {
      return content::Data
        { std::span<std::byte const>
          { _data.data() + offset
          , std::min (size, std::size (_data) - util::cast<size_t> (offset))
          }
        };
     }

    return content::Data{};
  }

  auto Vector::resize
    ( state::writing auto const& write_access
    , size_t size
    ) -> content::result::Resize
  {
    _data.resize (size, std::byte {0});
    _data.shrink_to_fit();
    xattr_set_capacity (write_access, XATTR_REPLACE);

    return content::result::resize::Success {size};
  }

  auto Vector::write
    ( state::writing auto const& write_access
    , content::Data const& data
    , off_t offset
    ) -> content::result::Write
  try
  {
    auto new_size {std::optional<size_t>{}};

    if ( auto const size {util::cast<size_t> (offset) + data.size()}
       ; std::size (_data) < size
       )
    {
      resize (write_access, size);

      new_size.emplace (size);
    }

    data.for_each_chunk
      ( [destination {std::next (std::begin (_data), offset)}]
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
      , "fuse::content::Vector::write: std::system_error: {}\n"
      , std::current_exception()
      );

    return content::result::write::Error {error.code().value()};
  }
  catch (std::length_error const&)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Vector::write: std::length_error: {}\n"
      , std::current_exception()
      );

    return content::result::write::Error {ENOMEM};
  }
  catch (std::bad_alloc const&)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::content::Vector::write: std::bad_alloc\n"
      );

    return content::result::write::Error {ENOMEM};
  }

  auto Vector::fallocate
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

    auto pos_at
      { [&] (auto n)
        {
          auto pos {std::begin (_data)};
          std::advance (pos, n);
          return pos;
        }
      };

    std::fill ( pos_at (offset)
              , pos_at (std::min (size, min_size))
              , std::byte {0}
              );

    return 0;
  }

  auto Vector::cleanup
    ( state::writing auto const&
    ) -> void
  {
    return;
  }
}

namespace mcs::fuse::content::implementation
{
  auto Vector::xattr_set_capacity
    ( state::writing auto const& write_access
    , int flags
    ) -> void
  {
    _inode->setxattr
      ( write_access
      , _xattr_key_capacity
      , _data.capacity()
      , flags
      );
  }
}
