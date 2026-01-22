// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/state/Access.hpp>
#include <mcs/fuse/state/Inode.hpp>
#include <mcs/fuse/state/inode/Common.hpp>
#include <mcs/fuse/state/inode/kind/File.hpp>
#include <optional>

namespace mcs::fuse::state
{
  template<is_content Content>
    struct Inode<Content, inode::kind::File<Content>> : private inode::Common
  {
  public:
    [[nodiscard]] constexpr Inode
      ( inode::kind::File<Content>
      , ::fuse_ino_t
      , uid_t
      , gid_t
      , mode_t
      );

    using inode::Common::ino;
    using inode::Common::stat;
    using inode::Common::fuse_entry_param;
    using inode::Common::inc_lookup;
    using inode::Common::dec_lookup;
    using inode::Common::inc_nlink;
    using inode::Common::dec_nlink;
    using inode::Common::setxattr;
    using inode::Common::getxattr;
    using inode::Common::listxattr_size;
    using inode::Common::listxattr;
    using inode::Common::removexattr;

    constexpr auto setattr_and_reply
      ( state::writing auto const&
      , ::fuse_req_t
      , struct stat* attr
      , int to_set
      ) -> void
      ;

    [[nodiscard]] constexpr auto size
      ( state::reading auto const&
      ) const noexcept -> size_t
      ;

    [[nodiscard]] auto data
      ( state::reading auto const&
      , size_t
      , off_t
      ) -> typename Content::Data
      ;

    auto write_and_reply
      ( state::writing auto const&
      , ::fuse_req_t
      , content::Data const&
      , off_t
      ) -> void
      ;

    auto trunc_success (state::writing auto const&, ::fuse_req_t) -> bool;

    [[nodiscard]] auto fallocate
      ( state::writing auto const&
      , int mode
      , off_t offset
      , off_t size
      ) -> int
      ;

    auto ioctl
      ( state::reading auto const&
      , ::fuse_req_t
      , unsigned int command
      , void *arg
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      ) -> void
      ;

    auto cleanup (state::writing auto const&) -> void;

  private:
    auto resize_success
      ( state::writing auto const&
      , ::fuse_req_t, size_t
      ) -> bool
      ;
    constexpr auto resized
      ( state::writing auto const&
      , size_t
      ) -> void
      ;

    Content _content;
  };
}

#include "detail/File.ipp"
