// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <ctime>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/divru.hpp>
#include <mcs/util/overloaded.hpp>
#include <sys/file.h>

namespace mcs::fuse::state
{
  template<is_content Content>
    constexpr Inode<Content, inode::kind::File<Content>>::Inode
      ( inode::kind::File<Content> file
      , ::fuse_ino_t ino
      , uid_t uid
      , gid_t gid
      , mode_t mode
      )
        : inode::Common {ino, uid, gid, nlink_t {1UL}, mode}
        , _content
          { file._content_state
          , this
          , *file._write_access
          , ino
          }
  {}

  template<is_content Content>
    constexpr auto Inode<Content, inode::kind::File<Content>>::setattr_and_reply
      ( state::writing auto const& write_access
      , ::fuse_req_t request
      , struct stat* attr
      , int to_set
      ) -> void
  {
    if (to_set & FUSE_SET_ATTR_SIZE)
    {
      if (! resize_success
             ( write_access
             , request
             , mcs::util::cast<size_t> (attr->st_size)
             )
         )
      {
        return;
      }
    }

    return inode::Common::setattr_and_reply
      ( write_access
      , request
      , attr
      , to_set
      );
  }

  template<is_content Content>
    constexpr auto Inode<Content, inode::kind::File<Content>>::size
      ( state::reading auto const& access_token
      ) const noexcept -> size_t
  {
    return _content.size (access_token);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::data
      ( state::reading auto const& access_token
      , size_t size
      , off_t offset
      ) -> typename Content::Data
  {
    return _content.data (access_token, size, offset);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::write_and_reply
      ( state::writing auto const& write_access
      , ::fuse_req_t request
      , content::Data const& data
      , off_t offset
      ) -> void
  {
    std::visit
      ( util::overloaded
        { [&] (content::result::write::Error const& error)
          {
            return reply::error (request, error.code);
          }
       ,  [&] (content::result::write::Success const& success)
          {
            if (success.new_size)
            {
              resized (write_access, *success.new_size);
            }

            return reply::write (request, data.size());
          }
        }
      , _content.write (write_access, data, offset)
      );
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::trunc_success
      ( state::writing auto const& write_access
      , ::fuse_req_t request
      ) -> bool
  {
    return resize_success (write_access, request, 0);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::fallocate
      ( state::writing auto const& write_access
      , int mode
      , off_t offset
      , off_t length
      ) -> int
  {
    return _content.fallocate (write_access, mode, offset, length);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::ioctl
      ( state::reading auto const& access_token
      , ::fuse_req_t request
      , unsigned int command
      , void *arg
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      ) -> void
  {
    if constexpr (has_ioctl<Content>)
    {
      return _content.ioctl
        ( access_token
        , request
        , command
        , arg
        , in_buf
        , in_buf_size
        , out_buf_size
        );
    }

    return reply::error (request, ENOSYS);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::resize_success
      ( state::writing auto const& write_access
      , ::fuse_req_t request
      , size_t size
      ) -> bool
  {
    return std::visit
      ( util::overloaded
        { [&] (content::result::resize::Success const&)
          {
            resized (write_access, size);

            return true;
          }
        , [&] (content::result::resize::Error const& error)
          {
            reply::error (request, error.code);

            return false;
          }
        }
      , _content.resize (write_access, size)
      );
  }

  template<is_content Content>
    constexpr auto Inode<Content, inode::kind::File<Content>>::resized
      ( state::writing auto const&
      , size_t size
      ) -> void
  {
    _stat.st_size = mcs::util::cast<off_t> (size);
    _stat.st_blocks = mcs::util::cast<blkcnt_t> (mcs::util::divru (size, 512UL));
    _stat.st_mtime = std::time (nullptr);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::File<Content>>::cleanup
      ( state::writing auto const& write_access
      ) -> void
  {
    return _content.cleanup (write_access);
  }
}
