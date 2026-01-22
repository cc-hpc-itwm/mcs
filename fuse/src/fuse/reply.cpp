// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstring>
#include <fmt/format.h>
#include <functional>
#include <mcs/Error.hpp>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <stdexcept>
#include <sys/uio.h>

namespace mcs::fuse::reply
{
  namespace
  {
    template<typename Fun, typename Description, typename...Args>
      auto non_zero_fails_with_minus_errno
        ( Fun&& fun
        , Description description
        , Args&&... args
        )
    {
      if ( auto eno
           { std::invoke
             ( std::forward<Fun> (fun)
             , std::forward<Args> (args)...
             )
           }
         ; eno != 0
       )
      {
        throw mcs::Error
          { fmt::format ("{}: {}", description, strerror (-eno))
          };
      }
    }
  }

  auto attr
    ( ::fuse_req_t request
    , struct stat const* attr
    , double attr_timeout
    ) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_attr
      , "fuse_reply_attr"
      , request
      , attr
      , attr_timeout
      );
  }

  auto buffer (::fuse_req_t request, std::span<char const> data) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_buf
      , "fuse_reply_buf"
      , request
      , data.data()
      , data.size()
      );
  }
  auto buffer (::fuse_req_t request, std::span<std::byte const> data) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_buf
      , "fuse_reply_buf"
      , request
      , util::cast<char const*> (data.data())
      , data.size()
      );
  }

  auto buffer (::fuse_req_t request, std::vector<char> data) -> void
  {
    buffer (request, std::span {data});
  }

  auto create
    ( ::fuse_req_t request
    , ::fuse_entry_param entry
    , ::fuse_file_info const* fi
    ) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_create
      , "fuse_reply_create"
      , request
      , std::addressof (entry)
      , fi
      );
  }

  auto data (::fuse_req_t request, content::Data const& data) -> void
  {
    if (data.chunks().empty())
    {
      return buffer (request, std::vector<char>{});
    }

    if (std::cmp_equal (data.chunks().size(), 1))
    {
      return buffer (request, data.chunks().front());
    }

    // \todo avoid copy, use reply::iov!?
    auto collected {std::vector<std::byte>{}};
    collected.reserve (data.size());

    auto position {std::back_inserter (collected)};

    data.for_each_chunk
      ( [&] (auto chunk)
        {
          std::ranges::copy (chunk, position);
        }
      );

    return buffer (request, collected);
  }

  auto entry (::fuse_req_t request, ::fuse_entry_param entry) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_entry
      , "fuse_reply_entry"
      , request
      , std::addressof (entry)
      );
  }

  auto error (::fuse_req_t request, int error) -> void
  {
    non_zero_fails_with_minus_errno
      ( [&]
        {
          // Sometimes "release" happens after the file system has
          // been unmounted and the socket has been closed already.
          //
          auto const rc {::fuse_reply_err (request, error)};

          if (rc == -EBADF)
          {
            return 0;
          }

          return rc;
        }
      , "fuse_reply_err"
      );
  }

  auto ioctl
    ( ::fuse_req_t request
    , int result
    , void const* buf
    , size_t size
    ) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_ioctl
      , "fuse_reply_ioctl"
      , request
      , result
      , buf
      , size
      );
  }

  auto lseek (::fuse_req_t request, off_t offset) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_lseek
      , "fuse_reply_lseek"
      , request
      , offset
      );
  }

  auto none (::fuse_req_t request) -> void
  {
    ::fuse_reply_none (request);
  }

  auto open (::fuse_req_t request, ::fuse_file_info const* fi) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_open
      , "fuse_reply_open"
      , request
      , fi
      );
  }

  auto readlink (::fuse_req_t request, char const* link) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_readlink
      , "fuse_reply_readlink"
      , request
      , link
      );
  }

  auto statfs (::fuse_req_t request, struct statvfs const* statvfs) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_statfs
      , "fuse_reply_statfs"
      , request
      , statvfs
      );
  }

  auto write (::fuse_req_t request, size_t count) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_write
      , "fuse_reply_write"
      , request
      , count
      );
  }

  auto xattr (::fuse_req_t request, size_t count) -> void
  {
    non_zero_fails_with_minus_errno
      ( ::fuse_reply_xattr
      , "fuse_reply_xattr"
      , request
      , count
      );
  }
}
