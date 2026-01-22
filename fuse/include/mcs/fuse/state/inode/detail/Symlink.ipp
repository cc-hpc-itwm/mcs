// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::fuse::state
{
  template<is_content Content>
    constexpr Inode<Content, inode::kind::Symlink>::Inode
      ( inode::kind::Symlink symlink
      , ::fuse_ino_t ino
      , uid_t uid
      , gid_t gid
      , mode_t mode
      )
        : inode::Common {ino, uid, gid, nlink_t {1UL}, mode}
        , _target_name {symlink._target_name}
  {}

  template<is_content Content>
    constexpr auto Inode<Content, inode::kind::Symlink>::target_name
      (
      ) const noexcept -> char const*
  {
    return _target_name.c_str();
  }
}
