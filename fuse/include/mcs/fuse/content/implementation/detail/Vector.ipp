// Copyright (C) 2025-2026 Fraunhofer ITWM
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
    , util::not_null<state::inode::Common>
    , state::writing auto const&
    , ::fuse_ino_t
    )
  {}

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
    ( state::writing auto const&
    , size_t size
    ) -> content::result::Resize
  {
    _data.resize (size, std::byte {0});
    _data.shrink_to_fit();

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

  auto Vector::cleanup
    ( state::writing auto const&
    ) -> void
  {
    return;
  }
}
