// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/Copy.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/fopen.hpp>
#include <mcs/util/syscall/copy_file_range_with_fallback_to_sendfile.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/lseek.hpp>
#include <mcs/util/syscall/read.hpp>
#include <mcs/util/syscall/write.hpp>

namespace mcs::util::copy
{
  FileLocation::FileLocation
    ( std::filesystem::path path
    , char const* mode
    , off_t offset
    )
      : _open_file {util::fopen (path, mode)}
      , _fd {util::syscall::fileno (_open_file.get())}
  {
    util::syscall::lseek (_fd, offset, SEEK_SET);
  }

  FileReadLocation::FileReadLocation
    ( std::filesystem::path path
    , off_t offset
    )
      : FileLocation {path, "rb", offset}
  {}

  FileWriteLocation::FileWriteLocation
    ( std::filesystem::path path
    , off_t offset
    )
      : FileLocation {path, "r+b", offset}
  {}
}

namespace mcs::util
{
  auto Copy::operator()
    ( copy::FileReadLocation const& from
    , copy::FileWriteLocation const& to
    , std::size_t size
    ) const -> std::size_t
  {
    auto bytes_copied {std::size_t {0}};
    auto bytes_left {size};

    while (bytes_copied < size)
    {
      auto const bytes_transferred
        { util::syscall::copy_file_range_with_fallback_to_sendfile
          ( from._fd
          , to._fd
          , bytes_left
	  )
        };

      bytes_copied += util::cast<std::size_t> (bytes_transferred);
      bytes_left -= util::cast<std::size_t> (bytes_transferred);
    }

    return bytes_copied;
  }
}

namespace mcs::util
{
  auto Copy::operator()
    ( std::byte const* from
    , copy::FileWriteLocation const& to
    , std::size_t size
    ) const -> std::size_t
  {
    auto bytes_copied {std::size_t {0}};
    auto bytes_left {size};

    while (bytes_copied < size)
    {
      auto const bytes_transferred
        { util::syscall::write (to._fd, from + bytes_copied, bytes_left)
        };

      bytes_copied += util::cast<std::size_t> (bytes_transferred);
      bytes_left -= util::cast<std::size_t> (bytes_transferred);
    }

    return bytes_copied;
  }
}

namespace mcs::util
{
  auto Copy::operator()
    ( copy::FileReadLocation const& from
    , std::byte* to
    , std::size_t size
    ) const -> std::size_t
  {
    auto bytes_copied {std::size_t {0}};
    auto bytes_left {size};

    while (bytes_copied < size)
    {
      auto const bytes_transferred
        { util::syscall::read (from._fd, to + bytes_copied, bytes_left)
        };

      bytes_copied += util::cast<std::size_t> (bytes_transferred);
      bytes_left -= util::cast<std::size_t> (bytes_transferred);
    }

    return bytes_copied;
  }
}
