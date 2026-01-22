// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/state/detail/operations.hpp>

namespace mcs::fuse::state
{
  template<is_content Content>
    constexpr auto operations() noexcept
  {
    return ::fuse_lowlevel_ops
      { .init = detail::init<Content>
      , .destroy = detail::destroy<Content>
      , .lookup = detail::lookup<Content>
      , .forget = detail::forget<Content>
      , .getattr = detail::getattr<Content>
      , .setattr = detail::setattr<Content>
      , .readlink = detail::readlink<Content>
      , .mknod = detail::mknod<Content>
      , .mkdir = detail::mkdir<Content>
      , .unlink = detail::unlink<Content>
      , .rmdir = detail::rmdir<Content>
      , .symlink = detail::symlink<Content>
      , .rename = detail::rename<Content>
      , .link = detail::link<Content>
      , .open = detail::open<Content>
      , .read = detail::read<Content>
      , .write = detail::write<Content>
      , .flush = nullptr
      , .release = detail::release<Content>
      , .fsync = nullptr
      , .opendir = detail::opendir<Content>
      , .readdir = detail::readdir<Content>
      , .releasedir = detail::releasedir<Content>
      , .fsyncdir = nullptr
      , .statfs = detail::statfs<Content>
      , .setxattr = detail::setxattr<Content>
      , .getxattr = detail::getxattr<Content>
      , .listxattr = detail::listxattr<Content>
      , .removexattr = detail::removexattr<Content>
      , .access = nullptr
      , .create = detail::create<Content>
      , .getlk = nullptr
      , .setlk = nullptr
      , .bmap = nullptr
      , .ioctl = detail::ioctl<Content>
      , .poll = nullptr
      , .write_buf = nullptr
      , .retrieve_reply = nullptr
      , .forget_multi = detail::forget_multi<Content>
      , .flock = nullptr
      , .fallocate = detail::fallocate<Content>
      , .readdirplus = detail::readdirplus<Content>
      , .copy_file_range = detail::copy_file_range<Content>
      , .lseek = detail::lseek<Content>
      , .tmpfile = nullptr
      , .statx = nullptr
      };
  }
}
