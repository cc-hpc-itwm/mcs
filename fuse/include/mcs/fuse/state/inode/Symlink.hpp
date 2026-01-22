// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/state/Inode.hpp>
#include <mcs/fuse/state/inode/Common.hpp>
#include <mcs/fuse/state/inode/kind/Symlink.hpp>
#include <string>

namespace mcs::fuse::state
{
  template<is_content Content>
    struct Inode<Content, inode::kind::Symlink> : private inode::Common
  {
  public:
    [[nodiscard]] constexpr Inode
      ( inode::kind::Symlink
      , ::fuse_ino_t
      , uid_t
      , gid_t
      , mode_t
      );

    using inode::Common::ino;
    using inode::Common::stat;
    using inode::Common::setattr_and_reply;
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
    using inode::Common::cleanup;

    [[nodiscard]] constexpr auto target_name() const noexcept -> char const*;

  private:
    std::string _target_name;
  };
}

#include "detail/Symlink.ipp"
