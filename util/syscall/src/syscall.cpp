// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cerrno>
#include <climits>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/Error.hpp>
#include <mcs/util/syscall/close.hpp>
#include <mcs/util/syscall/copy_file_range.hpp>
#include <mcs/util/syscall/copy_file_range_with_fallback_to_sendfile.hpp>
#include <mcs/util/syscall/dlclose.hpp>
#include <mcs/util/syscall/dlopen.hpp>
#include <mcs/util/syscall/dlsym.hpp>
#include <mcs/util/syscall/fileno.hpp>
#include <mcs/util/syscall/ftruncate.hpp>
#include <mcs/util/syscall/getegid.hpp>
#include <mcs/util/syscall/geteuid.hpp>
#include <mcs/util/syscall/getgid.hpp>
#include <mcs/util/syscall/getpid.hpp>
#include <mcs/util/syscall/getrlimit.hpp>
#include <mcs/util/syscall/getuid.hpp>
#include <mcs/util/syscall/hostname.hpp>
#include <mcs/util/syscall/lseek.hpp>
#include <mcs/util/syscall/mlock.hpp>
#include <mcs/util/syscall/mmap.hpp>
#include <mcs/util/syscall/munlock.hpp>
#include <mcs/util/syscall/munmap.hpp>
#include <mcs/util/syscall/pread.hpp>
#include <mcs/util/syscall/pwrite.hpp>
#include <mcs/util/syscall/read.hpp>
#include <mcs/util/syscall/realloc.hpp>
#include <mcs/util/syscall/sendfile.hpp>
#include <mcs/util/syscall/shm_open.hpp>
#include <mcs/util/syscall/shm_unlink.hpp>
#include <mcs/util/syscall/statfs.hpp>
#include <mcs/util/syscall/sysconf.hpp>
#include <mcs/util/syscall/write.hpp>
#include <mutex>
#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

namespace mcs::util::syscall
{
  Error::~Error() = default;

  namespace
  {
    auto syscall_error (int ec)
    {
      return std::system_error (std::error_code (ec, std::system_category()));
    }

    template<typename T, typename R>
      auto negative_one_fails_with_errno (R rc) -> T
    {
      if (rc == R (-1))
      {
        throw syscall_error (errno);
      }
      return rc;
    }

    template<typename Pointer>
      auto nullptr_fails_with_errno (Pointer pointer) -> Pointer
    {
      if (pointer == nullptr)
      {
        throw syscall_error (errno);
      }
      return pointer;
    }

    template<>
      auto negative_one_fails_with_errno<void, int> (int rc) -> void
    {
      if (rc == -1)
      {
        throw syscall_error (errno);
      }
    }

    auto MAP_FAILED_fails_with_errno (void* rc) -> void*
    {
      if (rc == MAP_FAILED)
      {
        throw syscall_error (errno);
      }
      return rc;
    }

    template<typename R>
      auto never_fails (R rc) -> R
    {
      return rc;
    }
  }

  auto close (int fd) -> void
  try
  {
    return negative_one_fails_with_errno<void> (::close (fd));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::close (fd = {})", fd)
        }
      );
  }

  auto copy_file_range
    ( int fd_in
    , off_t* off_in
    , int fd_out
    , off_t* off_out
    , size_t len
    , unsigned int flags
    ) -> ssize_t
  try
  {
    return negative_one_fails_with_errno<ssize_t>
      (::copy_file_range (fd_in, off_in, fd_out, off_out, len, flags));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::copy_file_range"
            " (fd_in = {}"
            ", off_in = {}@{}"
            ", fd_out = {}"
            ", off_out = {}@{}"
            ", len = {}"
            ", flags = {}"
            ")"
          , fd_in
          , off_in ? *off_in : off_t {-1}
          , cast<void*> (off_in)
          , fd_out
          , off_out ? *off_out : off_t {-1}
          , cast<void*> (off_out)
          , len
          , flags
          )
        }
      );
  }

  auto copy_file_range_with_fallback_to_sendfile
    ( int fd_in
    , int fd_out
    , size_t len
    ) -> ssize_t
  try
  {
    auto const r
      {::copy_file_range (fd_in, nullptr, fd_out, nullptr, len, 0)};

    if (r == -1 && errno == EXDEV)
    {
      return ::sendfile (fd_out, fd_in, nullptr, len);
    }

    return negative_one_fails_with_errno<ssize_t> (r);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::copy_file_range_with_fallback_to_sendfile"
            " (fd_in = {}"
            ", fd_out = {}"
            ", len = {}"
            ")"
          , fd_in
          , fd_out
          , len
          )
        }
      );
  }
}

namespace mcs::util::syscall::error
{
  DLError::DLError (std::string call, char const* reason)
    : mcs::Error {fmt::format ("dlerror ({}): {}", call, reason)}
    , _call {call}
    , _reason {reason}
  {}

  DLError::~DLError() = default;

  auto DLError::call() const noexcept -> std::string
  {
    return _call;
  }

  auto DLError::reason() const noexcept -> std::string
  {
    return _reason;
  }
}

namespace
{
  struct CheckDLError
  {
    CheckDLError (std::string call)
      : _call {call}
    {
      throw_if_there_is_a_dlerror();
    }
    template<typename Ret> auto operator() (Ret value) const -> Ret
    {
      throw_if_there_is_a_dlerror();

      return value;
    }

  private:
    std::string _call;

    auto throw_if_there_is_a_dlerror() const -> void
    {
      if (auto const reason {::dlerror()})
      {
        throw mcs::util::syscall::error::DLError {_call, reason};
      }
    }

    // \note dlerror() is specified to be not threadsafe. In fact,
    // it uses a single static buffer to print the error message
    // in. Thus, sequentialize all functions that call dlerror.
    //
    static std::mutex _guard;
    std::lock_guard<std::mutex> const _lock {_guard};
  };

  std::mutex CheckDLError::_guard;
}

namespace mcs::util::syscall
{
  auto dlopen (char const* filename, int flag) -> void*
  try
  {
    return CheckDLError {"dlopen"} (::dlopen (filename, flag));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::dlopen (filename = '{}', flag = {})"
                      , filename
                      , flag
                      )
        }
      );
  }
  auto dlsym (void* handle, char const* symbol) -> void*
  try
  {
    return CheckDLError {"dlsym"} (::dlsym (handle, symbol));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::dlsym (handle = {}, symbol = '{}')"
                      , handle
                      , symbol
                      )
        }
      );
  }
  auto dlclose (void* handle) -> int
  try
  {
    return CheckDLError {"dlclose"} (::dlclose (handle));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::dlclose (handle = {})", handle)
        }
      );
  }
}

namespace mcs::util::syscall
{
  auto fileno (FILE* stream) -> int
  try
  {
    return negative_one_fails_with_errno<int> (::fileno (stream));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::fileno (stream = {})", cast<void*> (stream))
        }
      );
  }

  auto ftruncate (int fd, off_t length) -> void
  try
  {
    return negative_one_fails_with_errno<void> (::ftruncate (fd, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::ftruncate (fd = {}, length = {})", fd, length)
        }
      );
  }

  auto getegid() -> gid_t
  {
    return never_fails (::getegid());
  }

  auto geteuid() -> gid_t
  {
    return never_fails (::geteuid());
  }

  auto getgid() -> gid_t
  {
    return never_fails (::getgid());
  }

  auto getpid() -> pid_t
  {
    return never_fails (::getpid());
  }

  auto getuid() -> uid_t
  {
    return never_fails (::getuid());
  }

  auto getrlimit (int resource) -> rlimit
  try
  {
    rlimit lim{};
    negative_one_fails_with_errno<void>
      (::getrlimit (resource, std::addressof (lim)));
    return lim;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::getrlimit (resource = {})", resource)
        }
      );
  }

  auto hostname() -> char const*
  try
  {
    static auto buffer {std::vector<char> (HOST_NAME_MAX + 1)};
    static auto name
      { std::invoke
        ( []
          {
            buffer.back() = '\0';

            negative_one_fails_with_errno<void>
              (::gethostname (buffer.data(), HOST_NAME_MAX));

            return buffer.data();
          }
        )
      };

    return name;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error {"syscall::hostname()"}
      );
  }

  auto lseek (int fd, off_t offset, int whence) -> off_t
  try
  {
    return negative_one_fails_with_errno<off_t> (::lseek (fd, offset, whence));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::lseek (fd = {}, offset = {}, whence = {})"
                      , fd
                      , offset
                      , whence
                      )
        }
      );
  }

  auto mlock (void const* addr, size_t length) -> void
  try
  {
    return negative_one_fails_with_errno<void> (::mlock (addr, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::mlock (addr = {}, length = {})", addr, length)
        }
      );
  }

  auto mmap (void* addr, size_t length, int prot, int flags, int fd, off_t offset) -> void*
  try
  {
    auto const page_size {util::syscall::sysconf (_SC_PAGE_SIZE)};

    if (offset % page_size != 0)
    {
      // \todo specific exception
      throw std::invalid_argument
        { fmt::format
          ( "mmap: offset must be a multiple of the page_size '{}'"
            " and offset {} = {} * {} + {}"
          , page_size
          , offset
          , offset / page_size
          , page_size
          , offset % page_size
          )
        };
    }

    return MAP_FAILED_fails_with_errno
      (::mmap (addr, length, prot, flags, fd, offset));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::mmap"
            " (addr = {}"
            ", length = {}"
            ", prot = {}"
            ", flags = {}"
            ", fd = {}"
            ", offset = {}"
            ")"
          , addr
          , length
          , prot
          , flags
          , fd
          , offset
          )
        }
      );
  }

  auto mmap_with_length_zero_allowed
    ( void* addr
    , size_t length
    , int prot
    , int flags
    , int fd
    , off_t offset
    ) -> void*
  try
  {
    if (std::cmp_equal (length, 0))
    {
      return nullptr;
    }

    return mmap (addr, length, prot, flags, fd, offset);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::mmap_with_length_zero_allowed"
            " (addr = {}"
            ", length = {}"
            ", prot = {}"
            ", flags = {}"
            ", fd = {}"
            ", offset = {}"
            ")"
          , addr
          , length
          , prot
          , flags
          , fd
          , offset
          )
        }
      );
  }

  auto munlock (void const* addr, size_t length) -> void
  try
  {
    return negative_one_fails_with_errno<void> (::munlock (addr, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::munlock (addr = {}, length = {})"
                      , addr
                      , length
                      )
        }
      );
  }

  auto munmap (void* addr, size_t length) -> void
  try
  {
    return negative_one_fails_with_errno<void> (::munmap (addr, length));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::munmap (addr = {}, length = {})"
                      , addr
                      , length
                      )
        }
      );
  }

  auto munmap_with_length_zero_allowed (void* addr, size_t length) -> void
  try
  {
    if (std::cmp_equal (length, 0))
    {
      if (addr != nullptr)
      {
        throw mcs::Error {"addr must be equal to nullptr"};
      }

      return;
    }

    return munmap (addr, length);
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::munmap_with_length_zero_allowed (addr = {}, length = {})"
          , addr
          , length
          )
        }
      );
  }

  auto pread (int fd, void* buf, size_t nbyte, off_t offset) -> ssize_t
  try
  {
    return negative_one_fails_with_errno<ssize_t>
      (::pread (fd, buf, nbyte, offset));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::pread (fd = {}, buf = {}, nbyte = {}, offset = {})"
          , fd
          , buf
          , nbyte
          , offset
          )
        }
      );
  }

  auto pwrite (int fd, void const* buf, size_t nbyte, off_t offset) -> ssize_t
  try
  {
    return negative_one_fails_with_errno<ssize_t>
      (::pwrite (fd, buf, nbyte, offset));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::pwrite (fd = {}, buf = {}, nbyte = {}, offset = {})"
          , fd
          , buf
          , nbyte
          , offset
          )
        }
      );
  }

  auto read (int fd, void* buf, size_t nbyte) -> ssize_t
  try
  {
    return negative_one_fails_with_errno<ssize_t> (::read (fd, buf, nbyte));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::read (fd = {}, buf = {}, nbyte = {})"
                      , fd
                      , buf
                      , nbyte
                      )
        }
      );
  }

  auto realloc (void* pointer, size_t size) -> void*
  try
  {
    return nullptr_fails_with_errno (::realloc (pointer, size));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::realloc (pointer = {}, size = {})"
                      , pointer
                      , size
                      )
        }
      );
  }

  auto sendfile
    ( int out_fd
    , int in_fd
    , off_t* offset
    , size_t count
    ) -> ssize_t
  try
  {
    return negative_one_fails_with_errno<ssize_t>
      ( ::sendfile (out_fd, in_fd, offset, count)
      );
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format
          ( "syscall::sendfile"
            " (out_fd = {}"
            ", in_fd = {}"
            ", offset = {}@{}"
            ", count = {}"
            ")"
          , out_fd
          , in_fd
          , offset ? *offset : off_t {-1}
          , cast<void*> (offset)
          , count
          )
        }
      );
  }

  auto shm_open (const char* name, int oflag, mode_t mode) -> int
  try
  {
    return negative_one_fails_with_errno<int> (::shm_open (name, oflag, mode));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::shm_open (name = '{}', oflag = {}, mode = {})"
                      , name
                      , oflag
                      , mode
                      )
        }
      );
  }

  auto shm_unlink (const char* name) -> void
  try
  {
    return negative_one_fails_with_errno<void> (::shm_unlink (name));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::shm_unlink (name = '{}')", name)
        }
      );
  }

  auto statfs (char const* path) -> struct statfs
  try
  {
    struct statfs stat{};
    negative_one_fails_with_errno<void>
      (::statfs (path, std::addressof (stat)));
    return stat;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::statfs (path = '{}')", path)
        }
      );
  }

  auto sysconf (int name) -> long
  try
  {
    errno = 0;
    auto const value {::sysconf (name)};
    if (value == -1 && errno != 0)
    {
      throw syscall_error (errno);
    }
    return value;
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ("syscall::sysconf (name = {})", name)
        }
      );
  }

  auto write (int fd, void const* buf, size_t nbyte) -> ssize_t
  try
  {
    return negative_one_fails_with_errno<ssize_t> (::write (fd, buf, nbyte));
  }
  catch (...)
  {
    std::throw_with_nested
      ( Error
        { fmt::format ( "syscall::write (fd = {}, buf = {}, nbyte = {}"
                      , fd
                      , buf
                      , nbyte
                      )
        }
      );
  }
}
