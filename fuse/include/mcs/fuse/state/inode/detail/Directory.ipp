// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/format.h>
#include <functional>
#include <iterator>
#include <mcs/Error.hpp>

namespace mcs::fuse::state
{
  template<is_content Content>
    constexpr Inode<Content, state::inode::kind::Directory>::Inode
      ( state::inode::kind::Directory
      , ::fuse_ino_t ino
      , uid_t uid
      , gid_t gid
      , mode_t mode
      )
        : inode::Common {ino, uid, gid, nlink_t {2UL}, mode}
  {}

  template<is_content Content>
    constexpr auto Inode<Content, state::inode::kind::Directory>::is_empty
      ( state::reading auto const&
      ) const -> bool
  {
    return _directories.empty() && _files.empty() && _symlinks.empty();
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::add_entry
      ( state::writing auto const& write_access
      , char const* name
      , state::Inode<Content, inode::kind::Directory>* inode
      ) -> void
  {
    if (!_directories.emplace (name, inode).second)
    {
      throw mcs::Error
        { fmt::format
          ( "fuse::inode::Directory: Can not add directory '{}'. Duplicate!?"
          , name
          )
        };
    }

    inc_nlink (write_access);
  }
  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::add_entry
      ( state::writing auto const&
      , char const* name
      , state::Inode<Content, inode::kind::File<Content>>* inode
      ) -> void
  {
    if (!_files.emplace (name, inode).second)
    {
      throw mcs::Error
        { fmt::format
          ( "fuse::inode::Directory: Can not add file '{}'. Duplicate!?"
          , name
          )
        };
    }
  }
  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::add_entry
      ( state::writing auto const&
      , char const* name
      , state::Inode<Content, inode::kind::Symlink>* inode
      ) -> void
  {
    if (!_symlinks.emplace (name, inode).second)
    {
      throw mcs::Error
        { fmt::format
          ( "fuse::inode::Directory: Can not add symlink '{}'. Duplicate!?"
          , name
          )
        };
    }
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::remove_entry
      ( state::writing auto const& write_access
      , Handle<inode::kind::Directory> directory
      ) -> void
  {
    _directories.erase (directory);

    dec_nlink (write_access);
  }
  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::remove_entry
      ( state::writing auto const&
      , Handle<inode::kind::File<Content>> file
      ) -> void
  {
    _files.erase (file);
  }
  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::remove_entry
      ( state::writing auto const&
      , Handle<inode::kind::Symlink> symlink
      ) -> void
  {
    _symlinks.erase (symlink);
  }

  template<is_content Content>
    auto Inode<Content, inode::kind::Directory>::directory_handle
      ( state::reading auto const& access_token
      ) const -> DirectoryHandle*
  {
    auto _directory_handle {new DirectoryHandle{}};

    _directory_handle->entries.reserve
      ( _directories.size()
      + _files.size()
      + _symlinks.size()
      );

    std::ranges::for_each
      ( _directories
      , [&] (auto directory)
        {
          _directory_handle->entries.emplace_back
            ( directory.name()
            , directory.inode()->fuse_entry_param (access_token)
            );
        }
      );

    std::ranges::for_each
      ( _files
      , [&] (auto file)
        {
          _directory_handle->entries.emplace_back
            ( file.name()
            , file.inode()->fuse_entry_param (access_token)
            );
        }
      );

    std::ranges::for_each
      ( _symlinks
      , [&] (auto symlink)
        {
          _directory_handle->entries.emplace_back
            ( symlink.name()
            , symlink.inode()->fuse_entry_param (access_token)
            );
        }
      );

    return _directory_handle;
  }
}
