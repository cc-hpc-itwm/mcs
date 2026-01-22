// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/fuse/state/Access.hpp>
#include <mcs/fuse/state/DirectoryHandle.hpp>
#include <mcs/fuse/state/Inode.hpp>
#include <mcs/fuse/state/inode/Common.hpp>
#include <mcs/fuse/state/inode/Kind.hpp>
#include <mcs/fuse/state/inode/directory/Entry.hpp>

#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_ORDERED_SET)
#include <set>
#endif

#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_UNORDERED_SET)
#include <unordered_set>
#endif

namespace mcs::fuse::state
{
  template<is_content Content>
    struct Inode<Content, inode::kind::Directory> : private inode::Common
  {
#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_ORDERED_SET)
    template<typename Kind>
      using Container
       = std::set < inode::directory::Entry<Content, Kind>
                  , typename inode::directory::Entry<Content, Kind>::Less
                  >
      ;
#endif

#if defined (MCS_FUSE_INODE_DIRECTORY_CONTAINER_UNORDERED_SET)
    template<typename Kind>
      using Container
       = std::unordered_set
          < inode::directory::Entry<Content, Kind>
          , typename inode::directory::Entry<Content, Kind>::Hash
          , typename inode::directory::Entry<Content, Kind>::Equal
          >
      ;
#endif

    [[nodiscard]] constexpr Inode
      ( inode::kind::Directory
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

    constexpr auto is_empty (state::reading auto const&) const -> bool;

    auto add_entry
      ( state::writing auto const&
      , char const* name
      , state::Inode<Content, inode::kind::Directory>*
      ) -> void
      ;
    auto add_entry
      ( state::writing auto const&
      , char const* name
      , state::Inode<Content, inode::kind::File<Content>>*
      ) -> void
      ;
    auto add_entry
      ( state::writing auto const&
      , char const* name
      , state::Inode<Content, inode::kind::Symlink>*
      ) -> void
      ;

    // \note It is okay to hand out iterators because container.erase
    // (iterator) does not invalidate other iterators and because the
    // handed out iterators do not live longer than the acquiring
    // unique lock.
    //
    template<typename Kind>
      using Handle = typename Container<Kind>::const_iterator;

    auto remove_entry
      ( state::writing auto const&
      , Handle<inode::kind::Directory>
      ) -> void
      ;
    auto remove_entry
      ( state::writing auto const&
      , Handle<inode::kind::File<Content>>
      ) -> void
      ;
    auto remove_entry
      ( state::writing auto const&
      , Handle<inode::kind::Symlink>
      ) -> void
      ;

    auto directory_handle
      ( state::reading auto const&
      ) const -> DirectoryHandle*
      ;

    template< typename UseDirectory
            , typename UseFile
            , typename UseSymlink
            , typename NotFound
            >
      requires (  std::invocable<UseDirectory, Handle<inode::kind::Directory>>
               && std::invocable<UseFile, Handle<inode::kind::File<Content>>>
               && std::invocable<UseSymlink, Handle<inode::kind::Symlink>>
               && std::invocable<NotFound>
               )
      auto visit_entry
        ( state::reading auto const&
        , char const* name
        , UseDirectory&& use_directory
        , UseFile&& use_file
        , UseSymlink&& use_symlink
        , NotFound&& not_found
        ) const -> void
    {
      if ( auto const directory {_directories.find (name)}
         ; directory != std::end (_directories)
         )
      {
        return std::invoke
          ( std::forward<UseDirectory> (use_directory)
          , directory
          );
      }

      if ( auto const file {_files.find (name)}
         ; file != std::end (_files)
         )
      {
        return std::invoke
          ( std::forward<UseFile> (use_file)
          , file
          );
      }

      if ( auto const symlink {_symlinks.find (name)}
         ; symlink != std::end (_symlinks)
         )
      {
        return std::invoke
          ( std::forward<UseSymlink> (use_symlink)
          , symlink
          );
      }

      return std::invoke (std::forward<NotFound> (not_found));
    }

    template<typename UseInode>
      requires (  std::invocable<UseInode, Handle<inode::kind::Directory>>
               && std::invocable<UseInode, Handle<inode::kind::File<Content>>>
               && std::invocable<UseInode, Handle<inode::kind::Symlink>>
               )
      auto with_entry
        ( state::reading auto const& access_token
        , ::fuse_req_t request
        , char const* name
        , UseInode&& use_inode
        ) const -> void
    {
      visit_entry
        ( access_token
        , name
        , std::forward<UseInode> (use_inode)
        , std::forward<UseInode> (use_inode)
        , std::forward<UseInode> (use_inode)
        , [&]
          {
            return reply::error (request, ENOENT);
          }
        );
    }

    template<typename UseFile>
      requires (std::invocable<UseFile, Handle<inode::kind::File<Content>>>)
      auto with_file_entry
        ( state::reading auto const&
        , ::fuse_req_t request
        , char const* name
        , UseFile&& use_file
        ) const -> void
    {
      if ( auto const file {_files.find (name)}
         ; file != std::end (_files)
         )
      {
        return std::invoke (std::forward<UseFile> (use_file), file);
      }

      return reply::error (request, ENOENT);
    }

    template<typename UseDirectory>
      requires (std::invocable<UseDirectory, Handle<inode::kind::Directory>>)
      auto with_directory_entry
        ( state::reading auto const&
        , ::fuse_req_t request
        , char const* name
        , UseDirectory&& use_directory
        ) const -> void
    {
      if ( auto const directory {_directories.find (name)}
         ; directory != std::end (_directories)
         )
      {
        return std::invoke
          ( std::forward<UseDirectory> (use_directory)
          , directory
          );
      }

      return reply::error (request, ENOENT);
    }

    template<typename NotFound>
      requires (std::invocable<NotFound>)
      auto without_entry
        ( state::reading auto const&
        , ::fuse_req_t request
        , char const* name
        , NotFound&& not_found
        ) const -> void
    {
      if (  _directories.contains (name)
         || _files.contains (name)
         || _symlinks.contains (name)
         )
      {
        return reply::error (request, EEXIST);
      }

      return std::invoke (std::forward<NotFound> (not_found));
    }

  private:
    Container<inode::kind::Directory> _directories;
    Container<inode::kind::File<Content>> _files;
    Container<inode::kind::Symlink> _symlinks;
  };
}

#include "detail/Directory.ipp"
