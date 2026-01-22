// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <functional>
#include <iterator>
#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/fuse/state/Access.hpp>
#include <mcs/fuse/state/Inode.hpp>
#include <mcs/fuse/state/inode/Directory.hpp>
#include <mcs/fuse/state/inode/File.hpp>
#include <mcs/fuse/state/inode/Symlink.hpp>
#include <mcs/util/Lock.hpp>
#include <mcs/util/lock/queue/Fast.hpp>
#include <mcs/util/not_null.hpp>
#include <mcs/util/syscall/getgid.hpp>
#include <mcs/util/syscall/getuid.hpp>
#include <memory>
#include <type_traits>
#include <utility>

#if defined (MCS_FUSE_STATE_CONTAINER_ORDERED_SET)
#include <set>
#endif

#if defined (MCS_FUSE_STATE_CONTAINER_UNORDERED_SET)
#include <unordered_set>
#endif

namespace mcs::fuse
{
  // Holds and owns inodes, separated into directories, files and
  // symbolic links. The libfuse operations are calling into the state.
  //
  // Access control is done on a course grained level in the state:
  // There may be multiple readers but only a single writer.
  //
  template<is_content Content>
    struct State
  {
    [[nodiscard]] State (util::not_null<typename Content::State>);

    auto init
      ( ::fuse_conn_info*
      ) -> void
      ;

    auto destroy
      (
      ) -> void
      ;

    auto lookup
      ( ::fuse_req_t
      , ::fuse_ino_t parent
      , char const* name
      ) -> void
      ;

    auto forget
      ( ::fuse_req_t
      , ::fuse_ino_t
      , uint64_t nlookup
      ) -> void
      ;

    auto getattr
      ( ::fuse_req_t
      , ::fuse_ino_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto setattr
      ( ::fuse_req_t
      , ::fuse_ino_t
      , struct stat*
      , int
      , ::fuse_file_info*
      ) -> void
      ;

    auto readlink
      ( ::fuse_req_t
      , ::fuse_ino_t
      ) -> void
      ;

    auto mknod
      ( ::fuse_req_t
      , ::fuse_ino_t parent
      , char const* name
      , mode_t
      , dev_t
      ) -> void
      ;

    auto mkdir
      ( ::fuse_req_t
      , ::fuse_ino_t parent
      , char const* name
      , mode_t
      ) -> void
      ;

    auto unlink
      ( ::fuse_req_t
      , ::fuse_ino_t parent
      , char const* name
      ) -> void
      ;

    auto rmdir
      ( ::fuse_req_t
      , ::fuse_ino_t parent
      , char const* name
      ) -> void
      ;

    auto symlink
      ( ::fuse_req_t
      , char const* link
      , ::fuse_ino_t
      , char const* name
      ) -> void
      ;

    auto rename
      ( ::fuse_req_t
      , ::fuse_ino_t parent
      , char const* name
      , ::fuse_ino_t newparent
      , char const* newname
      , unsigned int flags
      ) -> void
      ;

    auto link
      ( ::fuse_req_t
      , ::fuse_ino_t
      , ::fuse_ino_t newparent
      , char const* newname
      ) -> void
      ;

    auto open
      ( ::fuse_req_t
      , ::fuse_ino_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto read
      ( ::fuse_req_t
      , ::fuse_ino_t
      , size_t
      , off_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto write
      ( ::fuse_req_t
      , ::fuse_ino_t
      , char const*
      , size_t
      , off_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto release
      ( ::fuse_req_t
      , ::fuse_ino_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto opendir
      ( ::fuse_req_t
      , ::fuse_ino_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto readdir
      ( ::fuse_req_t
      , ::fuse_ino_t
      , size_t
      , off_t
      , ::fuse_file_info*
      ) -> void
    ;

    auto releasedir
      ( ::fuse_req_t
      , ::fuse_ino_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto setxattr
      ( ::fuse_req_t
      , ::fuse_ino_t
      , char const* key
      , char const* value
      , size_t
      , int flags
      ) -> void
      ;

    auto getxattr
      ( ::fuse_req_t
      , ::fuse_ino_t
      , char const* key
      , size_t
      )  -> void
      ;

    auto listxattr
      ( ::fuse_req_t
      , ::fuse_ino_t
      , size_t
      ) -> void
      ;

    auto removexattr
      ( ::fuse_req_t
      , ::fuse_ino_t
      , char const* key
      ) -> void
      ;

    auto statfs
      ( ::fuse_req_t
      , ::fuse_ino_t
      ) -> void
      ;

    auto create
      ( ::fuse_req_t
      , ::fuse_ino_t
      , char const* name
      , mode_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto ioctl
      ( ::fuse_req_t
      , ::fuse_ino_t
      , unsigned int command
      , void *arg
      , ::fuse_file_info*
      , unsigned flags
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      ) -> void
      ;

    auto forget_multi
      ( ::fuse_req_t
      , size_t
      , ::fuse_forget_data*
      ) -> void
      ;

    auto fallocate
      ( ::fuse_req_t
      , ::fuse_ino_t
      , int mode
      , off_t offset
      , off_t length
      , ::fuse_file_info*
      ) -> void
      ;

    auto readdirplus
      ( ::fuse_req_t
      , ::fuse_ino_t
      , size_t
      , off_t
      , ::fuse_file_info*
      ) -> void
      ;

    auto copy_file_range
      ( ::fuse_req_t
      , ::fuse_ino_t ino_in
      , off_t off_in
      , ::fuse_file_info *fi_in
      , ::fuse_ino_t ino_out
      , off_t off_out
      , ::fuse_file_info *fi_out
      , size_t len
      , int flags
      ) -> void
      ;

    auto lseek
      ( ::fuse_req_t
      , ::fuse_ino_t
      , off_t
      , int whence
      , ::fuse_file_info*
      ) -> void
      ;

    State (State const&) = delete;
    State (State&&) = delete;
    auto operator= (State const&) -> State& = delete;
    auto operator= (State&&) -> State& = delete;
    ~State() = default;

  private:
    util::lock::SharedMutex<util::lock::queue::Fast> _guard;
    util::not_null<typename Content::State> _content_state;

    [[nodiscard]] auto _read_access() -> state::access::Read;
    [[nodiscard]] auto _write_access() -> state::access::Write;

    uint64_t _next_ino {FUSE_ROOT_ID};
    std::unique_ptr<state::Inode<Content, state::inode::kind::Directory>> _root
      { std::make_unique<state::Inode<Content, state::inode::kind::Directory>>
        ( state::inode::kind::Directory{}
        , _next_ino++
        , util::syscall::getuid()
        , util::syscall::getgid()
        , S_IFDIR | 0755
        )
      };

#if defined (MCS_FUSE_STATE_CONTAINER_ORDERED_SET)
    struct Less
    {
      using is_transparent = std::true_type;

      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( state::Inode<Content, Kind>*
          , state::Inode<Content, Kind>*
          ) const noexcept -> bool
          ;
      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( ::fuse_ino_t
          , state::Inode<Content, Kind>*
          ) const noexcept -> bool
          ;
      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( state::Inode<Content, Kind>*
          , ::fuse_ino_t
          ) const noexcept -> bool
          ;
    };
    std::set
      < state::Inode<Content, state::inode::kind::Directory>*
      , Less
      > _directories {{{_root.get()}}}
      ;
    std::set
      < state::Inode<Content, state::inode::kind::File<Content>>*
      , Less
      > _files{}
      ;
    std::set
      < state::Inode<Content, state::inode::kind::Symlink>*
      , Less
      > _symlinks{}
      ;
#endif

#if defined (MCS_FUSE_STATE_CONTAINER_UNORDERED_SET)
    struct Equal
    {
      using is_transparent = std::true_type;

      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( state::Inode<Content, Kind>*
          , state::Inode<Content, Kind>*
          ) const noexcept -> bool
          ;
      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( ::fuse_ino_t
          , state::Inode<Content, Kind>*
          ) const noexcept -> bool
          ;
      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( state::Inode<Content, Kind>*
          , ::fuse_ino_t
          ) const noexcept -> bool
          ;
    };
    struct Hash
    {
      using is_transparent = std::true_type;
      using is_transparent_key_equal = Equal;

      template<typename Kind>
        requires (state::inode::is_kind<Content, Kind>)
        [[nodiscard]] constexpr auto operator()
          ( state::Inode<Content, Kind>*
          ) const noexcept -> std::size_t
          ;
      [[nodiscard]] constexpr auto operator()
        ( ::fuse_ino_t
        ) const noexcept -> std::size_t
        ;
    };
    std::unordered_set
      < state::Inode<Content, state::inode::kind::Directory>*
      , Hash
      , Equal
      > _directories {{{_root.get()}}}
      ;
    std::unordered_set
      < state::Inode<Content, state::inode::kind::File<Content>>*
      , Hash
      , Equal
      > _files{}
      ;
    std::unordered_set
      < state::Inode<Content, state::inode::kind::Symlink>*
      , Hash
      , Equal
      > _symlinks{}
      ;
#endif

    auto _forget
      ( state::writing auto const&
      , ::fuse_ino_t
      , uint64_t nlookup
      ) -> void
      ;

    template<typename Kind>
      requires (state::inode::is_kind<Content, Kind>)
      [[nodiscard]] auto create_and_add_entry
        ( state::writing auto const&
        , state::Inode<Content, state::inode::kind::Directory>* parent
        , ::fuse_ctx const*
        , char const* name
        , Kind&&
        , mode_t
        ) -> ::fuse_entry_param
      ;

    template< typename UseDirectory
            , typename UseFile
            , typename UseSymlink
            , typename NotFound
            >
      requires (  std::invocable<UseDirectory, state::Inode<Content, state::inode::kind::Directory>*>
               && std::invocable<UseFile, state::Inode<Content, state::inode::kind::File<Content>>*>
               && std::invocable<UseSymlink, state::Inode<Content, state::inode::kind::Symlink>*>
               && std::invocable<NotFound>
               )
      auto visit_inode
        ( state::reading auto const&
        , ::fuse_ino_t ino
        , UseDirectory&& use_directory
        , UseFile&& use_file
        , UseSymlink&& use_symlink
        , NotFound&& not_found
        ) const
    {
      if ( auto const directory {_directories.find (ino)}
         ; directory != std::end (_directories)
         )
      {
        return std::invoke
          ( std::forward<UseDirectory> (use_directory)
          , *directory
          );
      }

      if ( auto const file {_files.find (ino)}
         ; file != std::end (_files)
         )
      {
        return std::invoke (std::forward<UseFile> (use_file), *file);
      }

      if ( auto const symlink {_symlinks.find (ino)}
         ; symlink != std::end (_symlinks)
         )
      {
        return std::invoke (std::forward<UseSymlink> (use_symlink), *symlink);
      }

      return std::invoke (std::forward<NotFound> (not_found));
    }

    template<typename UseInode, typename NotFound>
      requires (  std::invocable<UseInode, state::Inode<Content, state::inode::kind::Directory>*>
               && std::invocable<UseInode, state::Inode<Content, state::inode::kind::File<Content>>*>
               && std::invocable<UseInode, state::Inode<Content, state::inode::kind::Symlink>*>
               && std::invocable<NotFound>
               )
      auto if_inode_else
        ( state::reading auto const& access_token
        , ::fuse_ino_t ino
        , UseInode&& use_inode
        , NotFound&& not_found
        ) const
    {
      return visit_inode
        ( access_token
        , ino
        , std::forward<UseInode> (use_inode)
        , std::forward<UseInode> (use_inode)
        , std::forward<UseInode> (use_inode)
        , std::forward<NotFound> (not_found)
        );
    }

    template<typename UseInode>
      requires (  std::invocable<UseInode, state::Inode<Content, state::inode::kind::Directory>*>
               && std::invocable<UseInode, state::Inode<Content, state::inode::kind::File<Content>>*>
               && std::invocable<UseInode, state::Inode<Content, state::inode::kind::Symlink>*>
               )
      auto with_inode
        ( state::reading auto const& access_token
        , ::fuse_req_t request
        , ::fuse_ino_t ino
        , UseInode&& use_inode
        ) const -> void
    {
      return if_inode_else
        ( access_token
        , ino
        , std::forward<UseInode> (use_inode)
        , [&]
          {
            return reply::error (request, ENOENT);
          }
        );
    }

    template<typename UseDirectory>
      requires (std::invocable<UseDirectory, state::Inode<Content, state::inode::kind::Directory>*>)
      auto with_directory
        ( state::reading auto const&
        , ::fuse_req_t request
        , ::fuse_ino_t ino
        , UseDirectory&& use_directory
        ) const -> void
    {
      if ( auto const directory {_directories.find (ino)}
         ; directory != std::end (_directories)
         )
      {
        return std::invoke
          ( std::forward<UseDirectory> (use_directory)
          , *directory
          );
      }

      return reply::error (request, ENOENT);
    }

    template<typename UseFile>
      requires (std::invocable<UseFile, state::Inode<Content, state::inode::kind::File<Content>>*>)
      auto with_file
        ( state::reading auto const&
        , ::fuse_req_t request
        , ::fuse_ino_t ino
        , UseFile&& use_file
        ) const -> void
    {
      if ( auto const file {_files.find (ino)}
         ; file != std::end (_files)
         )
      {
        return std::invoke (std::forward<UseFile> (use_file), *file);
      }

      return reply::error (request, ENOENT);
    }

    template<typename UseSymlink>
      requires (std::invocable<UseSymlink, state::Inode<Content, state::inode::kind::Symlink>*>)
      auto with_symlink
        ( state::reading auto const&
        , ::fuse_req_t request
        , ::fuse_ino_t ino
        , UseSymlink&& use_symlink
        ) const -> void
    {
      if ( auto const symlink {_symlinks.find (ino)}
         ; symlink != std::end (_symlinks)
         )
      {
        return std::invoke
          ( std::forward<UseSymlink> (use_symlink)
          , *symlink
          );
      }

      return reply::error (request, ENOENT);
    }
  };
}

#include "detail/State.ipp"
