// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/content/Data.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <span>
#include <vector>

namespace mcs::fuse::reply
{
  auto attr (::fuse_req_t, struct stat const*, double attr_timeout = 0.0) -> void;
  auto buffer (::fuse_req_t, std::span<char const>) -> void;
  auto buffer (::fuse_req_t, std::span<std::byte const>) -> void;
  auto buffer (::fuse_req_t, std::vector<char>) -> void;
  auto create (::fuse_req_t, ::fuse_entry_param, ::fuse_file_info const*) -> void;
  auto data (::fuse_req_t, content::Data const&) -> void;
  auto entry (::fuse_req_t, ::fuse_entry_param) -> void;
  auto error (::fuse_req_t, int error) -> void;
  auto ioctl (::fuse_req_t, int, void const*, size_t) -> void;
  auto lseek (::fuse_req_t, off_t) -> void;
  auto none (::fuse_req_t) -> void;
  auto open (::fuse_req_t, ::fuse_file_info const*) -> void;
  auto readlink (::fuse_req_t, char const* link) -> void;
  auto statfs (::fuse_req_t, struct statvfs const*) -> void;
  auto write (::fuse_req_t, size_t count) -> void;
  auto xattr (::fuse_req_t, size_t count) -> void;
}
