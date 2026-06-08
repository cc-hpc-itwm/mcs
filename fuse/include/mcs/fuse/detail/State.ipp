// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <linux/falloc.h>
#include <linux/fs.h>
#include <mcs/Error.hpp>
#include <mcs/fuse/content/Data.hpp>
#include <mcs/fuse/state/DirectoryHandle.hpp>
#include <mcs/util/buffer/Bytes.hpp>
#include <mcs/util/cast.hpp>
#include <memory>
#include <span>
#include <utility>

namespace mcs::fuse
{
  template<is_content Content>
    State<Content>::State
      ( util::not_null<typename Content::State> content_state
      )
        : _content_state {content_state}
  {}
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::_read_access() -> state::access::Read
  {
    return state::access::Read {_guard};
  }
  template<is_content Content>
    auto State<Content>::_write_access() -> state::access::Write
  {
    return state::access::Write {_guard};
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::init
    ( ::fuse_conn_info* conn
    ) -> void
  {
    return _content_state->fuse_init (conn);
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::destroy
      (
      ) -> void
  {
    return _content_state->fuse_destroy();
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::lookup
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->with_entry
            ( write_access
            , request
            , name
            , [&] (auto entry)
              {
                entry->inode()->inc_lookup (write_access);

                return reply::entry
                  ( request
                  , entry->inode()->fuse_entry_param (write_access)
                  );
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::forget
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , uint64_t nlookup
      ) -> void
  {
    auto const write_access {_write_access()};

    _forget (write_access, ino, nlookup);

    return reply::none (request);
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::getattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = fi;

    auto const read_access {_read_access()};

    return with_inode
      ( read_access
      , request
      , ino
      , [&] (auto inode)
        {
          auto stat {inode->stat (read_access)};

          return reply::attr (request, std::addressof (stat));
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::setattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , struct stat* attr
      , int to_set
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = fi;

    auto const write_access {_write_access()};

    return with_inode
      ( write_access
      , request
      , ino
      , [&] (auto inode)
        {
          return inode->setattr_and_reply
            ( write_access
            , request
            , attr
            , to_set
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::readlink
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      ) -> void
  {
    return with_symlink
      ( _read_access()
      , request
      , ino
      , [&] (auto symlink)
        {
          return reply::readlink (request, symlink->target_name());
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::mknod
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , mode_t mode
      , dev_t rdev
      ) -> void
  {
    std::ignore = rdev;

    if (! (S_ISREG (mode) || S_ISLNK (mode)))
    {
      return reply::error (request, EOPNOTSUPP);
    }

    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->without_entry
            ( write_access
            , request
            , name
            , [&]
              {
                return reply::entry
                  ( request
                  , S_ISREG (mode) ? create_and_add_entry
                    ( write_access
                    , directory
                    , ::fuse_req_ctx (request)
                    , name
                    , state::inode::kind::File<Content>
                      { _content_state
                      , std::addressof (write_access)
                      }
                    , mode
                    )
                  : S_ISLNK (mode) ? create_and_add_entry
                    ( write_access
                    , directory
                    , ::fuse_req_ctx (request)
                    , name
                    , state::inode::kind::Symlink {nullptr}
                    , mode
                    )
                  : throw mcs::Error
                    { "INCONSISENT: mknod: not reg, symlink"
                    }
                  );
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::mkdir
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , mode_t mode
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->without_entry
            ( write_access
            , request
            , name
            , [&]
              {
                return reply::entry
                  ( request
                  , create_and_add_entry
                    ( write_access
                    , directory
                    , ::fuse_req_ctx (request)
                    , name
                    , state::inode::kind::Directory{}
                    , mode | S_IFDIR
                    )
                  );
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::unlink
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->visit_entry
            ( write_access
            , name
            , [&] (auto)
              {
                return reply::error (request, EISDIR);
              }
            , [&] (auto file)
              {
                file->inode()->dec_nlink (write_access);
                directory->remove_entry (write_access, file);

                return reply::error (request, 0);
              }
            , [&] (auto symlink)
              {
                symlink->inode()->dec_nlink (write_access);
                directory->remove_entry (write_access, symlink);

                return reply::error (request, 0);
              }
            , [&]
              {
                return reply::error (request, ENOENT);
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::rmdir
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->with_directory_entry
            ( write_access
            , request
            , name
            , [&] (auto entry)
              {
                if (!entry->inode()->is_empty (write_access))
                {
                  return reply::error (request, ENOTEMPTY);
                }

                // Bring the removed directory's nlink to 0: one
                // decrement for the parent's named reference, one for
                // the directory's own "." self-reference.
                //
                entry->inode()->dec_nlink (write_access);
                entry->inode()->dec_nlink (write_access);
                directory->remove_entry (write_access, entry);

                return reply::error (request, 0);
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::symlink
      ( ::fuse_req_t request
      , char const* target_name
      , ::fuse_ino_t parent
      , char const* link_name
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->without_entry
            ( write_access
            , request
            , link_name
            , [&]
              {
                return reply::entry
                  ( request
                  , create_and_add_entry
                    ( write_access
                    , directory
                    , ::fuse_req_ctx (request)
                    , link_name
                    , state::inode::kind::Symlink {target_name}
                    , 0777 | S_IFLNK
                    )
                  );
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::rename
      ( ::fuse_req_t request
      , ::fuse_ino_t source_parent
      , char const* source_name
      , ::fuse_ino_t target_parent
      , char const* target_name
      , unsigned int flags
      ) -> void
  {
    constexpr unsigned int supported_flags
      { RENAME_NOREPLACE
      | RENAME_EXCHANGE
      };

    if (flags & ~supported_flags)
    {
      return reply::error (request, EINVAL);
    }

    if ((flags & RENAME_NOREPLACE) && (flags & RENAME_EXCHANGE))
    {
      return reply::error (request, EINVAL);
    }

    auto const write_access {_write_access()};

    // \todo Check for ancestor: If source is a directory, then
    // target_parent must not be a descendant of source. This requires
    // parent traversal which the current data structure does not
    // efficiently support.

    auto const move_to_nonexisting
      { [&] ( auto source_directory, auto source
            , auto target_directory
            )
        {
          if (flags & RENAME_EXCHANGE)
          {
            return ENOENT;
          }

          auto source_inode {source->inode()};

          source_directory->remove_entry
            ( write_access
            , source
            );
          target_directory->add_entry
            ( write_access
            , target_name
            , source_inode
            );

          return 0;
        }
      };

    auto const move_to_existing
      { [&] ( auto source_directory, auto source
            , auto target_directory, auto target
            )
        {
          auto source_inode {source->inode()};
          auto target_inode {target->inode()};

          if constexpr
            (std::is_same_v<decltype (source_inode), decltype (target_inode)>)
          {
            if (  source_directory == target_directory
               && source_inode == target_inode
               )
            {
              return 0;
            }
          }

          if (flags & RENAME_NOREPLACE)
          {
            return EEXIST;
          }

          // For non-exchange replacement the target inode is
          // displaced and loses its directory entry. For files and
          // symlinks that means nlink 1 -> 0. For a directory that
          // means nlink 2 -> 0 (the parent's named reference and the
          // directory's own "." self-reference both go away,
          // analogous to rmdir).
          //
          if (! (flags & RENAME_EXCHANGE))
          {
            target_inode->dec_nlink (write_access);

            if constexpr
              ( std::is_same_v
                  < std::remove_pointer_t<decltype (target_inode)>
                  , state::Inode<Content, state::inode::kind::Directory>
                  >
              )
            {
              target_inode->dec_nlink (write_access);
            }
          }

          target_directory->remove_entry
            ( write_access
            , target
            );
          source_directory->remove_entry
            ( write_access
            , source
            );
          target_directory->add_entry
            ( write_access
            , target_name
            , source_inode
            );

          if (flags & RENAME_EXCHANGE)
          {
            source_directory->add_entry
              ( write_access
              , source_name
              , target_inode
              );
          }

          return 0;
        }
      };

    return with_directory
      ( write_access
      , request
      , source_parent
      , [&] (auto source_directory)
        {
          return with_directory
            ( write_access
            , request
            , target_parent
            , [&] (auto target_directory)
              {
                return source_directory->visit_entry
                  ( write_access
                  , source_name
                  , [&] (auto source_is_directory)
                    {
                      return target_directory->visit_entry
                        ( write_access
                        , target_name
                        , [&] (auto target_is_directory)
                          {
                            // RENAME_EXCHANGE allows non-empty directories
                            if (  !(flags & RENAME_EXCHANGE)
                               && !target_is_directory->inode()
                                     ->is_empty (write_access)
                               )
                            {
                              return reply::error (request, ENOTEMPTY);
                            }

                            return reply::error
                              ( request
                              , move_to_existing
                                ( source_directory, source_is_directory
                                , target_directory, target_is_directory
                                )
                              );
                          }
                        , [&] (auto target_is_file)
                          {
                            if (flags & RENAME_EXCHANGE)
                            {
                              return reply::error
                                ( request
                                , move_to_existing
                                  ( source_directory, source_is_directory
                                  , target_directory, target_is_file
                                  )
                                );
                            }
                            return reply::error (request, ENOTDIR);
                          }
                        , [&] (auto target_is_symlink)
                          {
                            if (flags & RENAME_EXCHANGE)
                            {
                              return reply::error
                                ( request
                                , move_to_existing
                                  ( source_directory, source_is_directory
                                  , target_directory, target_is_symlink
                                  )
                                );
                            }
                            return reply::error (request, ENOTDIR);
                          }
                        , [&] /* target does not exist */
                          {
                            return reply::error
                              ( request
                              , move_to_nonexisting
                                ( source_directory, source_is_directory
                                , target_directory
                                )
                              );
                          }
                        );
                    }
                  , [&] (auto source_is_file)
                    {
                      return target_directory->visit_entry
                        ( write_access
                        , target_name
                        , [&] (auto target_is_directory)
                          {
                            if (flags & RENAME_EXCHANGE)
                            {
                              // RENAME_EXCHANGE allows non-empty directories
                              return reply::error
                                ( request
                                , move_to_existing
                                  ( source_directory, source_is_file
                                  , target_directory, target_is_directory
                                  )
                                );
                            }
                            return reply::error (request, EISDIR);
                          }
                        , [&] (auto target_is_file)
                          {
                            return reply::error
                              ( request
                              , move_to_existing
                                ( source_directory, source_is_file
                                , target_directory, target_is_file
                                )
                              );
                          }
                        , [&] (auto target_is_symlink)
                          {
                            return reply::error
                              ( request
                              , move_to_existing
                                ( source_directory, source_is_file
                                , target_directory, target_is_symlink
                                )
                              );
                          }
                        , [&] /* target_does_not_exist */
                          {
                            return reply::error
                              ( request
                              , move_to_nonexisting
                                ( source_directory, source_is_file
                                , target_directory
                                )
                              );
                          }
                        );
                    }
                  , [&] (auto source_is_symlink)
                    {
                      return target_directory->visit_entry
                        ( write_access
                        , target_name
                        , [&] (auto target_is_directory)
                          {
                            if (flags & RENAME_EXCHANGE)
                            {
                              // RENAME_EXCHANGE allows non-empty directories
                              return reply::error
                                ( request
                                , move_to_existing
                                  ( source_directory, source_is_symlink
                                  , target_directory, target_is_directory
                                  )
                                );
                            }
                            return reply::error (request, EISDIR);
                          }
                        , [&] (auto target_is_file)
                          {
                            return reply::error
                              ( request
                              , move_to_existing
                                ( source_directory, source_is_symlink
                                , target_directory, target_is_file
                                )
                              );
                          }
                        , [&] (auto target_is_symlink)
                          {
                            return reply::error
                              ( request
                              , move_to_existing
                                ( source_directory, source_is_symlink
                                , target_directory, target_is_symlink
                                )
                              );
                          }
                        , [&] /* target_does_not_exist */
                          {
                            return reply::error
                              ( request
                              , move_to_nonexisting
                                ( source_directory, source_is_symlink
                                , target_directory
                                )
                              );
                          }
                        );
                    }
                  , [&] /* source does not exist */
                    {
                      return reply::error (request, ENOENT);
                    }
                  );
              }
            );
         }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::link
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_ino_t target_parent
      , char const* target_name
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_inode
      ( write_access
      , request
      , ino
      , [&] (auto source)
        {
          return with_directory
            ( write_access
            , request
            , target_parent
            , [&] (auto target_directory)
              {
                return target_directory->without_entry
                  ( write_access
                  , request
                  , target_name
                  , [&]
                    {
                      source->inc_nlink (write_access);
                      source->inc_lookup (write_access);

                      target_directory->add_entry
                        ( write_access
                        , target_name
                        , source
                        );

                      return reply::entry
                        ( request
                        , source->fuse_entry_param (write_access)
                        );
                    }
                  );
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::open
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) -> void
  {
    // \todo would the use of fi avoid searches?

    if (fi->flags & O_TRUNC)
    {
      if ((fi->flags & O_RDWR) || (fi->flags & O_WRONLY))
      {
        auto const write_access {_write_access()};

         return with_file
           ( write_access
           , request
           , ino
           , [&] (auto file)
             {
               if (file->trunc_success (write_access, request))
               {
                 return reply::open (request, fi);
               }
             }
           );
      }
    }
    else
    {
      return with_file
        ( _read_access()
        , request
        , ino
        , [&] (auto)
          {
            return reply::open (request, fi);
          }
        );
    }
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::read
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = fi;

    auto const read_access {_read_access()};

    return with_file
      ( read_access
      , request
      , ino
      , [&] (auto file)
        {
          // Named variable of type Content::Data to keep state alive.
          //
          auto const data {file->data (read_access, size, offset)};

          return reply::data (request, data);
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::write
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* buf
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = fi;

    auto const write_access {_write_access()};

    return with_file
      ( write_access
      , request
      , ino
      , [&] (auto file)
        {
          return file->write_and_reply
            ( write_access
            , request
            , content::Data {std::as_bytes (std::span {buf, size})}
            , offset
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::release
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = ino;
    std::ignore = fi;

    return reply::error (request, 0);
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::opendir
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) -> void
  {
    auto const read_access {_read_access()};

    return with_directory
      ( read_access
      , request
      , ino
      , [&] (auto directory)
        {
          fi->fh = util::cast<uint64_t>
            ( directory->directory_handle (read_access)
            );

          return reply::open (request, fi);
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::readdir
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = ino;

    auto const* directory_handle
      { util::cast<state::DirectoryHandle const*> (fi->fh)
      };

    if (!directory_handle)
    {
      return reply::error (request, EBADF);
    }

    auto const memory {util::buffer::Bytes {size}};
    auto* buffer {memory.data<char>().data()};
    auto bytes_used {size_t {0}};

    for ( auto bytes_remaining {size}
        ; offset < util::cast<off_t> (directory_handle->entries.size())
        ; ++offset
        )
    {
      auto const& entry
        { directory_handle->entries.at (util::cast<std::size_t> (offset))
        };

      auto const entry_size
        { ::fuse_add_direntry
          ( request
          , buffer + bytes_used
          , bytes_remaining
          , entry.name.c_str()
          , std::addressof (entry.entry_param.attr)
          , offset + 1
          )
        };

      if (entry_size > bytes_remaining)
      {
        break;
      }

      bytes_remaining -= entry_size;
      bytes_used += entry_size;
    }

    return reply::buffer (request, std::span {buffer, bytes_used});
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::releasedir
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = ino;

    delete util::cast<state::DirectoryHandle*> (fi->fh);

    return reply::error (request, 0);
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::setxattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* key
      , char const* value
      , size_t size
      , int flags
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_inode
      ( write_access
      , request
      , ino
      , [&] (auto inode)
        {
          return reply::error
            ( request
            , inode->setxattr
              ( write_access
              , key
              , std::span {value, size}
              , flags
              )
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::getxattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* key
      , size_t size
      ) -> void
  {
    auto const read_access {_read_access()};

    return with_inode
      ( read_access
      , request
      , ino
      , [&] (auto inode)
        {
          if (auto const value {inode->getxattr (read_access, key)})
          {
            if (size == 0)
            {
              return reply::xattr (request, value->size());
            }

            if (size < value->size())
            {
              return reply::error (request, ERANGE);
            }

            return reply::buffer (request, *value);
          }

          return reply::error (request, ENODATA);
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::listxattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      ) -> void
  {
    auto const read_access {_read_access()};

    return with_inode
      ( read_access
      , request
      , ino
      , [&] (auto inode)
        {
          if (size == 0)
          {
            return reply::xattr
              ( request
              , inode->listxattr_size (read_access)
              );
          }

          if (size < inode->listxattr_size (read_access))
          {
            return reply::error (request, ERANGE);
          }

          return reply::buffer (request, inode->listxattr (read_access));
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::removexattr
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , char const* key
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_inode
      ( write_access
      , request
      , ino
      , [&] (auto inode)
        {
          return reply::error
            ( request
            , inode->removexattr (write_access, key)
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::statfs
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      ) -> void
  {
    std::ignore = ino;

    auto const read_access {_read_access()};

    struct statvfs statvfs{};

    statvfs.f_bsize = _content_state->f_bsize (read_access);
    statvfs.f_frsize = _content_state->f_frsize (read_access);
    statvfs.f_blocks = _content_state->f_blocks (read_access);
    statvfs.f_bfree = _content_state->f_bfree (read_access);
    statvfs.f_bavail = _content_state->f_bavail (read_access);
    statvfs.f_files = _directories.size() + _files.size() + _symlinks.size();
    statvfs.f_ffree = std::numeric_limits<fsfilcnt_t>::max() - _next_ino;
    statvfs.f_favail = std::numeric_limits<fsfilcnt_t>::max() - _next_ino;
    statvfs.f_fsid = 0;                  // \todo parameter!?
    statvfs.f_flag = 0;                  // \todo from options!?
    statvfs.f_namemax = PATH_MAX;

    return reply::statfs (request, std::addressof (statvfs));
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::create
      ( ::fuse_req_t request
      , ::fuse_ino_t parent
      , char const* name
      , mode_t mode
      , ::fuse_file_info* fi
      ) -> void
  {
    auto const write_access {_write_access()};

    return with_directory
      ( write_access
      , request
      , parent
      , [&] (auto directory)
        {
          return directory->without_entry
            ( write_access
            , request
            , name
            , [&]
              {
                return reply::create
                  ( request
                  , create_and_add_entry
                    ( write_access
                    , directory
                    , ::fuse_req_ctx (request)
                    , name
                    , state::inode::kind::File<Content>
                      { _content_state
                      , std::addressof (write_access)
                      }
                    , mode
                    )
                  , fi
                  );
              }
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::ioctl
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , unsigned int command
      , void *arg
      , ::fuse_file_info* fi
      , unsigned flags
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      ) -> void
  {
    std::ignore = fi;

    if (flags & FUSE_IOCTL_COMPAT)
    {
      return reply::error (request, ENOSYS);
    }

    auto const read_access {_read_access()};

    return with_file
      ( read_access
      , request
      , ino
      , [&] (auto file)
        {
          return file->ioctl
            ( read_access
            , request
            , command
            , arg
            , in_buf
            , in_buf_size
            , out_buf_size
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::forget_multi
      ( ::fuse_req_t request
      , size_t count
      , ::fuse_forget_data* forgets
      ) -> void
  {
    auto const write_access {_write_access()};

    std::ranges::for_each
      ( std::span {forgets, count}
      , [&] (auto forget)
        {
          _forget (write_access, forget.ino, forget.nlookup);
        }
      );

    return reply::none (request);
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::fallocate
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , int mode
      , off_t offset
      , off_t length
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = fi;

    auto const write_access {_write_access()};

    return with_file
      ( write_access
      , request
      , ino
      , [&] (auto file)
        {
          return reply::error
            ( request
            , file->fallocate (write_access, mode, offset, length)
            );
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::readdirplus
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , size_t size
      , off_t offset
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = ino;

    auto* directory_handle
      { util::cast<state::DirectoryHandle*> (fi->fh)
      };

    if (!directory_handle)
    {
      return reply::error (request, EBADF);
    }

    auto const memory {util::buffer::Bytes {size}};
    auto* buffer {memory.data<char>().data()};
    auto bytes_used {size_t {0}};
    auto bytes_remaining {size};

    auto const write_access {_write_access()};

    auto const add_entry
      { [&] () -> std::optional<size_t>
        {
          if (! (offset < util::cast<off_t> (directory_handle->entries.size())))
          {
            return {};
          }

          auto& entry
            { directory_handle->entries[util::cast<std::size_t> (offset)]
            };

          return if_inode_else
             ( write_access
             , entry.entry_param.ino
             , [&] (auto inode) -> std::optional<size_t>
               {
                 inode->inc_lookup (write_access);

                 entry.entry_param = inode->fuse_entry_param (write_access);

                 auto const entry_size
                   { ::fuse_add_direntry_plus
                       ( request
                       , buffer + bytes_used
                       , bytes_remaining
                       , entry.name.c_str()
                       , std::addressof (entry.entry_param)
                       , offset + 1
                       )
                   };

                 if (entry_size > bytes_remaining)
                 {
                   return {};
                 }

                 return entry_size;
               }
             , []() noexcept -> std::optional<size_t>
               {
                 return {};
               }
             );
        }
      };

    while (auto const entry_size {add_entry()})
    {
      bytes_remaining -= *entry_size;
      bytes_used += *entry_size;
      ++offset;
    }

    return reply::buffer (request, std::span {buffer, bytes_used});
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::copy_file_range
      ( ::fuse_req_t request
      , ::fuse_ino_t ino_in
      , off_t off_in
      , ::fuse_file_info *fi_in
      , ::fuse_ino_t ino_out
      , off_t off_out
      , ::fuse_file_info *fi_out
      , size_t len
      , int flags
      ) -> void
  {
    std::ignore = fi_in;
    std::ignore = fi_out;
    std::ignore = flags;

    auto const write_access {_write_access()};

    return with_file
      ( write_access
      , request
      , ino_in
      , [&] (auto file_in)
        {
          if (ino_in == ino_out)
          {
            // Named variable of type Content::Data to keep state alive.
            //
            auto const data {file_in->data (write_access, len, off_in)};

            if ( auto const data_size
                 { util::cast<off_t>
                     ( static_cast<content::Data const&> (data).size()
                     )
                 }
               ; std::max (off_in, off_out)
                 < std::min (off_in + data_size, off_out + data_size)
               )
            {
              //      overlaps               off_in     off_in + data_size
              // in                          [----------)
              // out  no       [----------)  :          :
              //      no          [----------)          :
              //      yes              [----------)     :
              //      yes                    [----------)
              //      yes                    :     [----------)
              //      no                     :          [----------)
              //      no                     :          :      [----------)
              //
              return reply::error (request, EINVAL);
            }

            return file_in->write_and_reply
              ( write_access
              , request
              , data
              , off_out
              );
          }
          else
          {
            return with_file
              ( write_access
              , request
              , ino_out
              , [&] (auto file_out)
                {
                  auto const data_in
                    { file_in->data (write_access, len, off_in)
                    };

                  return file_out->write_and_reply
                    ( write_access
                    , request
                    , data_in
                    , off_out
                    );
                }
              );
          }
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::lseek
      ( ::fuse_req_t request
      , ::fuse_ino_t ino
      , off_t offset
      , int whence
      , ::fuse_file_info* fi
      ) -> void
  {
    std::ignore = fi;

    auto const read_access {_read_access()};

    return with_file
      ( read_access
      , request
      , ino
      , [&] (auto file)
        {
          auto const file_size {file->size (read_access)};

          switch (whence)
          {
            break; case SEEK_SET:
              return reply::lseek (request, offset);

            break; case SEEK_CUR:
              return reply::error (request, EOPNOTSUPP);

            break; case SEEK_END:
              return reply::lseek (request, util::cast<off_t> (file_size) + offset);

            break; case SEEK_HOLE:
              return reply::lseek (request, util::cast<off_t> (file_size));

            break; case SEEK_DATA:
              return reply::lseek (request, offset);

            break; default:
              return reply::error (request, EINVAL);
          }
        }
      );
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    auto State<Content>::_forget
      ( state::writing auto const& write_access
      , ::fuse_ino_t ino
      , uint64_t nlookup
      ) -> void
  {
    auto do_erase
      { [&] (auto& container, auto inode_pos)
        {
          auto inode {*inode_pos};

          // An inode may only be destroyed when the kernel no longer
          // caches it (nlookup == 0) AND no directory entry still
          // refers to it (st_nlink == 0). Otherwise the parent
          // directory's name map would dangle: the kernel can send
          // FORGET on cache pressure while the entry is still live in
          // the server's namespace, and a subsequent LOOKUP would
          // dereference a freed pointer.
          //
          auto const remaining_lookups
            { inode->dec_lookup (write_access, nlookup)
            };

          if (  std::cmp_equal (0, remaining_lookups)
             && std::cmp_equal (0, inode->stat (write_access).st_nlink)
             )
          {
            container.erase (inode_pos);

            inode->cleanup (write_access);

            delete inode;
          }
        }
      };

    if ( auto const directory {_directories.find (ino)}
       ; directory != std::end (_directories)
       )
    {
      return do_erase (_directories, directory);
    }

    if ( auto const file {_files.find (ino)}
       ; file != std::end (_files)
       )
    {
      return do_erase (_files, file);
    }

    if ( auto const symlink {_symlinks.find (ino)}
       ; symlink != std::end (_symlinks)
       )
    {
      return do_erase (_symlinks, symlink);
    }
  }
}

namespace mcs::fuse
{
  template<is_content Content>
    template<typename Kind>
      requires (state::inode::is_kind<Content, Kind>)
      auto State<Content>::create_and_add_entry
        ( state::writing auto const& write_access
        , state::Inode<Content, state::inode::kind::Directory>* parent
        , ::fuse_ctx const* context
        , char const* name
        , Kind&& kind
        , mode_t mode
        ) -> ::fuse_entry_param
  {
    auto do_create
      { [&] (auto& container)
        {
          auto [entry_pos, inserted]
            { container.emplace
              ( new state::Inode<Content, Kind>
                { std::forward<Kind> (kind)
                , _next_ino++
                , context->uid
                , context->gid
                , mode
                }
              )
            };

          if (!inserted)
          {
            throw mcs::Error {"FATAL: Could not create inode."};
          }

          auto entry {*entry_pos};

          parent->add_entry (write_access, name, entry);

          return entry->fuse_entry_param (write_access);
        }
      };

    if constexpr (std::is_same_v<Kind, state::inode::kind::Directory>)
    {
      return do_create (_directories);
    }

    if constexpr (std::is_same_v<Kind, state::inode::kind::File<Content>>)
    {
      return do_create (_files);
    }

    if constexpr (std::is_same_v<Kind, state::inode::kind::Symlink>)
    {
      return do_create (_symlinks);
    }
  }
}

#if defined (MCS_FUSE_STATE_CONTAINER_ORDERED_SET)
namespace mcs::fuse
{
  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
      constexpr auto State<Content>::Less::operator()
        ( state::Inode<Content, Kind>* lhs
        , state::Inode<Content, Kind>* rhs
        ) const noexcept -> bool
  {
    return std::less{} (lhs->ino(), rhs->ino());
  }
  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
    constexpr auto State<Content>::Less::operator()
      ( ::fuse_ino_t lhs
      , state::Inode<Content, Kind>* rhs
      ) const noexcept -> bool
  {
    return std::less{} (lhs, rhs->ino());
  }
  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
      constexpr auto State<Content>::Less::operator()
        ( state::Inode<Content, Kind>* lhs
        , ::fuse_ino_t rhs
        ) const noexcept -> bool
  {
    return std::less{} (lhs->ino(), rhs);
  }
}
#endif

#if defined (MCS_FUSE_STATE_CONTAINER_UNORDERED_SET)
namespace mcs::fuse
{
  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
      constexpr auto State<Content>::Equal::operator()
        ( state::Inode<Content, Kind>* lhs
        , state::Inode<Content, Kind>* rhs
        ) const noexcept -> bool
  {
    return std::equal_to{} (lhs->ino(), rhs->ino());
  }
  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
    constexpr auto State<Content>::Equal::operator()
      ( ::fuse_ino_t lhs
      , state::Inode<Content, Kind>* rhs
      ) const noexcept -> bool
  {
    return std::equal_to{} (lhs, rhs->ino());
  }
  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
      constexpr auto State<Content>::Equal::operator()
        ( state::Inode<Content, Kind>* lhs
        , ::fuse_ino_t rhs
        ) const noexcept -> bool
  {
    return std::equal_to{} (lhs->ino(), rhs);
  }

  template<is_content Content>
    template<typename Kind>
    requires (state::inode::is_kind<Content, Kind>)
      constexpr auto State<Content>::Hash::operator()
        ( state::Inode<Content, Kind>* inode
        ) const noexcept -> std::size_t
  {
    return std::hash<::fuse_ino_t>{} (inode->ino());
  }
  template<is_content Content>
    constexpr auto State<Content>::Hash::operator()
      ( ::fuse_ino_t ino
      ) const noexcept -> std::size_t
  {
    return std::hash<::fuse_ino_t>{} (ino);
  }
}
#endif
