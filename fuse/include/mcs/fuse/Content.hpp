// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/fuse/content/Data.hpp>
#include <mcs/fuse/content/result/Resize.hpp>
#include <mcs/fuse/content/result/Write.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/state/Access.hpp>
#include <mcs/fuse/state/inode/Common.hpp>
#include <mcs/util/not_null.hpp>
#include <span>
#include <stddef.h>
#include <sys/types.h>
#include <type_traits>

namespace mcs::fuse
{
  // Content of a file. Bytes in a range [0..size()).
  //
  template<typename Content>
    concept is_content
      =  // Content gets access to the content state, the inode and an
         // unique ino. It has write access into the inode, e.g. to
         // modify the extended attributes. Synchronization of the
         // content state is managed by the fuse state.
         //
         std::is_constructible_v
          < Content
          , util::not_null<typename Content::State>
          , util::not_null<state::inode::Common>
          , state::access::Write const&
          , ::fuse_ino_t
          >
         // Content state interaction with the init/destroy of the
         // libfuse.
         //
      && requires ( util::not_null<typename Content::State> state
                  , state::access::Read const& access_token
                  , ::fuse_conn_info* conn
                  )
         {
           { state->fuse_init (conn)
           } -> std::convertible_to<void>
           ;
           { state->fuse_destroy()
           } -> std::convertible_to<void>
           ;
           // statvfs:
           // Returns: f_bsize  - Filesystem block size
           // Returns: f_frsize - Fragment size
           // Returns: f_blocks - Size of fs in f_frsize units
           // Returns: f_bfree  - Number of free blocks
           // Returns: f_bavail - Number of free blocks for unprivileged users
           { state->f_bsize (access_token)
           } -> std::convertible_to<unsigned long>
           ;
           { state->f_frsize (access_token)
           } -> std::convertible_to<unsigned long>
           ;
           { state->f_blocks (access_token)
           } -> std::convertible_to<fsblkcnt_t>
           ;
           { state->f_bfree (access_token)
           } -> std::convertible_to<fsblkcnt_t>
           ;
           { state->f_bavail (access_token)
           } -> std::convertible_to<fsblkcnt_t>
           ;
         }
      && requires ( Content const& content
                  , state::access::Read const& access_token
                  , size_t size
                  , off_t offset
                  )
         {
           // Returns: The size of the content.
           //
           { content.size (access_token)
           } -> std::convertible_to<size_t>
           ;
           // Returns: The intersection of [offset, offset + size) and
           // the data in [0..size()). It holds:
           //   - data has at most size many bytes
           //     : !(size < data.size())
           //   - if the offset is outside of the data, then the span is empty
           //     : !(offset < size()) ==> data.size() == 0
           //
           { content.data (access_token, size, offset)
           } -> std::convertible_to<typename Content::Data>
           ;
         }
      && requires ( Content& content
                  , state::access::Write const& write_access
                  , content::Data const& data
                  , size_t size
                  , off_t offset
                  , int mode
                  , off_t length
                  )
         {
           // Post: The data has been stored in the content, that
           //       means that a subsequent read will receive the
           //       stored data. The content has grown if necessary.
           //       : !(size() < offset + data.size())
           //       : write (data, offset)
           //         ==> data == data (data.size(), offset)
           //
           { content.write (write_access, data, offset)
           } -> std::convertible_to<content::result::Write>
           ;

           // Post: There is enough space for size many bytes.
           //       : !(size() < size)
           //
           // If size < size(), then resize forgets the bytes after size.
           // If size() < size, then resize sets the bytes after size to 0.
           //
           { content.resize (write_access, size)
           } -> std::convertible_to<content::result::Resize>
           ;

           // See man 2 fallocate
           //
           { content.fallocate (write_access, mode, offset, length)
           } -> std::convertible_to<int>
           ;

           // Called immediately before destruction
           //
           { content.cleanup (write_access)
           } -> std::convertible_to<void>
           ;
         }
      ;

  // IOCTL executor: Can not change the content but read properties.
  //
  template<typename Content>
    concept has_ioctl = requires
      ( Content const& content
      , state::access::Read const& access_token
      , ::fuse_req_t request
      , unsigned int command
      , void* arg
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      )
        {
          { content.ioctl
            ( access_token
            , request
            , command
            , arg
            , in_buf
            , in_buf_size
            , out_buf_size
            )
          } -> std::convertible_to<void>
          ;
        }
    ;
}
