// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <ctime>
#include <functional>
#include <mcs/fuse/state/inode/Common.hpp>

namespace mcs::fuse::state::inode
{
  Common::Common
    ( ::fuse_ino_t ino
    , uid_t uid
    , gid_t gid
    , nlink_t nlink
    , mode_t mode
    ) noexcept
      : _stat
        { std::invoke
          ( [&]() noexcept
            {
              struct stat stat{};
              stat.st_ino = ino;
              stat.st_mode = mode;
              stat.st_nlink = nlink;
              stat.st_uid = uid;
              stat.st_gid = gid;
              stat.st_atime = stat.st_mtime = stat.st_ctime
                = std::time (nullptr)
                ;
              return stat;
            }
          )
        }
  {}
}
