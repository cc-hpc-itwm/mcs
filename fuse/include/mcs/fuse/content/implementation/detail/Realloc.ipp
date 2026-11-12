// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iterator>
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
  Realloc::Realloc
    ( util::not_null<State>
    , util::not_null<state::inode::Common>
    , state::writing auto const&
    , ::fuse_ino_t
    )
  {}

  auto Realloc::resize
    ( state::writing auto const&
    , size_t size
    ) -> content::result::Resize
  {
    if (size == 0UL)
    {
      std::free (_data);

      _data = nullptr;
      _size = 0UL;
      _capacity = 0UL;

      return content::result::resize::Success {size};
    }

    auto const new_capacity
      { std::invoke
        ( [&]() noexcept -> std::optional<std::size_t>
          {
            if (_capacity < size)
            {
              return std::max (size, grow (_capacity));
            }

            if (grow (size) < _capacity)
            {
              auto _new_capacity {shrink (_capacity)};

              while (grow (size) < _new_capacity)
              {
                _new_capacity = shrink (_new_capacity);
              }

              return _new_capacity;
            }

            return {};
          }
        )
      };

    if (new_capacity)
    {
      _data = mcs::util::cast<std::byte*>
        ( util::syscall::realloc (_data, *new_capacity)
        );

      _capacity = *new_capacity;
    }

    if (_size < size)
    {
      std::ranges::fill
        ( std::span {_data + _size, _data + size}
        , std::byte {0}
        );
    }

    _size = size;

    return content::result::resize::Success {size};
  }

  auto Realloc::write
    ( state::writing auto const&
    , content::Data const& data
    , off_t offset
    ) -> content::result::Write
  try
  {
    auto const write_offset {mcs::util::cast<size_t> (offset)};
    auto const new_size
      { std::invoke
        ( [&]() -> std::optional<size_t>
          {
            if ( auto const size {write_offset + data.size()}
               ; _size < size
               )
            {
              if (size > _capacity)
              {
                auto const new_capacity {std::max (size, grow (_capacity))};

                _data = mcs::util::cast<std::byte*>
                  ( util::syscall::realloc (_data, new_capacity)
                  );

                _capacity = new_capacity;
              }

              return size;
            }

            return {};
          }
        )
      };

    if (_size < write_offset)
    {
      std::ranges::fill
        ( std::span {_data + _size, _data + write_offset}
        , std::byte {0}
        );
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

    if (new_size)
    {
      _size = *new_size;
    }

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
    return n - n / 3UL;
  }
}
