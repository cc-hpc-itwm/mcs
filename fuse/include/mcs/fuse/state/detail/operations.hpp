// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <cstdlib>
#include <exception>
#include <mcs/fuse/State.hpp>
#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <utility>

namespace mcs::fuse::state::detail
{
  template< is_content Content
          , typename Fun
          , typename Description
          , typename... Args
          >
    requires (std::invocable<Fun, State<Content>*, Args...>)
    auto lift
      ( Fun&& fun
      , Description description
      , State<Content>* state
      , Args&&... args
      ) noexcept
  try
  {
    std::invoke (std::forward<Fun> (fun), state, std::forward<Args> (args)...);
  }
  catch (...)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::lift: {}: {}\n"
      , description
      , std::current_exception()
      );

    std::exit (EXIT_FAILURE);
  }

  template< is_content Content
          , typename Fun
          , typename Description
          , typename... Args
          >
    requires (std::invocable<Fun, State<Content>*, ::fuse_req_t, Args...>)
    auto lift_and_reply_EIO_on_exception
      ( Fun&& fun
      , Description description
      , ::fuse_req_t request
      , Args&&... args
      ) noexcept
  try
  {
    std::invoke
      ( std::forward<Fun> (fun)
      , util::cast<State<Content>*> (::fuse_req_userdata (request))
      , request
      , std::forward<Args> (args)...
      );
  }
  catch (...)
  {
    util::FMT::print_noexcept
      ( stderr
      , "fuse::lift_and_reply_EIO_on_exception: {}: {}\n"
      , description
      , std::current_exception()
      );

    reply::error (request, EIO);
  }

  template<is_content Content>
    auto init
     ( void* user_data
     , ::fuse_conn_info* conn
     ) noexcept -> void
  {
    lift
      ( &State<Content>::init
      , "init"
      , util::cast<State<Content>*> (user_data)
      , conn
      );
  }

  template<is_content Content>
    auto destroy
     ( void* user_data
     ) noexcept -> void
  {
    lift
      ( &State<Content>::destroy
      , "destroy"
      , util::cast<State<Content>*>  (user_data)
      );
  }

  template<is_content Content>
    auto lookup
     ( ::fuse_req_t request
     , ::fuse_ino_t parent
     , char const* name
     ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::lookup
      , "lookup"
      , request
      , parent
      , name
      );
  }

  template<is_content Content>
    auto forget
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , uint64_t nlookup
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::forget
      , "forget"
      , request
      , ino
      , nlookup
      );
  }

  template<is_content Content>
    auto getattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::getattr
      , "getattr"
      , request
      , ino
      , fi
      );
  }

  template<is_content Content>
    auto setattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , struct stat* attr
      , int to_set
      , ::fuse_file_info* fi
      )
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::setattr
      , "setattr"
      , request
      , ino
      , attr
      , to_set
      , fi
      );
  }

  template<is_content Content>
    auto readlink
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      )
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::readlink
      , "readlink"
      , request
      , ino
      );
  }

  template<is_content Content>
    auto mknod
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , mode_t mode
      , dev_t rdev
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::mknod
      , "mknod"
      , request
      , parent
      , name
      , mode
      , rdev
      );
  }

  template<is_content Content>
    auto mkdir
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , mode_t mode
      )
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::mkdir
      , "mkdir"
      , request
      , parent
      , name
      , mode
      );
  }

  template<is_content Content>
    auto unlink
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::unlink
      , "unlink"
      , request
      , parent
      , name
      );
  }

  template<is_content Content>
    auto rmdir
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::rmdir
      , "rmdir"
      , request
      , parent
      , name
      );
  }

  template<is_content Content>
    auto symlink
      ( ::fuse_req_t request
      , char const* link
      , ::fuse_ino_t parent
      , char const* name
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::symlink
      , "symlink"
      , request
      , link
      , parent
      , name
      );
  }

  template<is_content Content>
    auto rename
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , ::fuse_ino_t newparent
      , char const* newname
      , unsigned int flags
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::rename
      , "rename"
      , request
      , parent
      , name
      , newparent
      , newname
      , flags
      );
  }

  template<is_content Content>
    auto link
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_ino_t newparent
      , char const* newname
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::link
      , "link"
      , request
      , ino
      , newparent
      , newname
      );
  }

  template<is_content Content>
    auto open
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info *fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::open
      , "open"
      , request
      , ino
      , fi
      );
  }

  template<is_content Content>
    auto read
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::read
      , "read"
      , request
      , ino
      , size
      , offset
      , fi
      );
  }

  template<is_content Content>
    auto write
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* buf
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::write
      , "write"
      , request
      , ino
      , buf
      , size
      , offset
      , fi
      );
  }

  template<is_content Content>
    auto release
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::release
      , "release"
      , request
      , ino
      , fi
      );
  }

  template<is_content Content>
    auto opendir
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::opendir
      , "opendir"
      , request
      , ino
      , fi
      );
  }

  template<is_content Content>
    auto readdir
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::readdir
      , "readdir"
      , request
      , ino
      , size
      , offset
      , fi
      );
  }

  template<is_content Content>
    auto releasedir
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::releasedir
      , "releasedir"
      , request
      , ino
      , fi
      );
  }

  template<is_content Content>
    auto setxattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* key
      , char const* value
      , size_t size
      , int flags
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::setxattr
      , "setxattr"
      , request
      , ino
      , key
      , value
      , size
      , flags
      );
  }

  template<is_content Content>
    auto getxattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* key
      , size_t size
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::getxattr
      , "getxattr"
      , request
      , ino
      , key
      , size
      );
  }

  template<is_content Content>
    auto listxattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::listxattr
      , "listxattr"
      , request
      , ino
      , size
      );
  }

  template<is_content Content>
    auto removexattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* key
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::removexattr
      , "removexattr"
      , request
      , ino
      , key
      );
  }

  template<is_content Content>
    auto statfs
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::statfs
      , "statfs"
      , request
      , ino
      );
  }

  template<is_content Content>
    auto create
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , mode_t mode
      , ::fuse_file_info* fi
      )
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::create
      , "create"
      , request
      , parent
      , name
      , mode
      , fi
      );
  }

  template<is_content Content>
    auto ioctl
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , unsigned int command
      , void *arg
      , ::fuse_file_info* fi
      , unsigned flags
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      )
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::ioctl
      , "ioctl"
      , request
      , ino
      , command
      , arg
      , fi
      , flags
      , in_buf
      , in_buf_size
      , out_buf_size
      );
  }

  template<is_content Content>
    auto forget_multi
      ( ::fuse_req_t request
      , size_t count
      , ::fuse_forget_data* forgets
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::forget_multi
      , "forget_multi"
      , request
      , count
      , forgets
      );
  }

  template<is_content Content>
    auto fallocate
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , int mode
      , off_t offset
      , off_t length
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::fallocate
      , "fallocate"
      , request
      , ino
      , mode
      , offset
      , length
      , fi
      );
  }

  template<is_content Content>
    auto readdirplus
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::readdirplus
      , "readdirplus"
      , request
      , ino
      , size
      , offset
      , fi
      );
  }

  template<is_content Content>
    auto copy_file_range
      ( ::fuse_req_t request
      , ::fuse_ino_t ino_in
      , off_t off_in
      , ::fuse_file_info* fi_in
      , ::fuse_ino_t ino_out
      , off_t off_out
      , ::fuse_file_info* fi_out
      , size_t len
      , int flags
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::copy_file_range
      , "copy_file_range"
      , request
      , ino_in
      , off_in
      , fi_in
      , ino_out
      , off_out
      , fi_out
      , len
      , flags
      );
  }

  template<is_content Content>
    auto lseek
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , off_t off
      , int whence
      , ::fuse_file_info* fi
      ) noexcept -> void
  {
    lift_and_reply_EIO_on_exception<Content>
      ( &State<Content>::lseek
      , "lseek"
      , request
      , ino
      , off
      , whence
      , fi
      );
  }
}
