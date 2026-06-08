// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <iterator>
#include <mcs/fuse/fuse/reply.hpp>
#include <mcs/serialization/OArchive.hpp>
#include <mcs/serialization/save.hpp>
#include <mcs/util/cast.hpp>
#include <mcs/util/syscall/getgid.hpp>
#include <mcs/util/syscall/getuid.hpp>
#include <memory>
#include <ranges>
#include <sys/xattr.h>

namespace mcs::fuse::state::inode
{
  constexpr auto Common::ino() const noexcept -> ::fuse_ino_t
  {
    return _stat.st_ino;
  }

  constexpr auto Common::stat
    ( state::reading auto const&
    ) const noexcept -> struct stat
  {
    return _stat;
  }

  constexpr auto Common::fuse_entry_param
    ( state::reading auto const&
    , double timeout_attr
    , double timeout_entry
    ) const noexcept -> ::fuse_entry_param
  {
    ::fuse_entry_param entry_param{};
    entry_param.attr = _stat;
    entry_param.ino = entry_param.attr.st_ino;
    entry_param.attr_timeout = timeout_attr;
    entry_param.entry_timeout = timeout_entry;
    return entry_param;
  }

  constexpr auto Common::inc_lookup
    ( state::writing auto const&
    ) noexcept -> void
  {
    ++_nlookup;
  }
  constexpr auto Common::dec_lookup
    ( state::writing auto const&
    , uint64_t count
    ) noexcept -> uint64_t
  {
    // Clamp against underflow: the kernel may issue more forgets
    // than we counted lookups (e.g. when entries are evicted after
    // a rename that displaced an inode the kernel still tracks).
    //
    return _nlookup -= std::min (count, _nlookup);
  }

  constexpr auto Common::inc_nlink
    ( state::writing auto const&
    ) noexcept -> void
  {
    ++_stat.st_nlink;
  }
  constexpr auto Common::dec_nlink
    ( state::writing auto const&
    ) noexcept -> void
  {
    --_stat.st_nlink;
  }

  constexpr auto Common::setattr_and_reply
    ( state::writing auto const&
    , ::fuse_req_t request
    , struct stat* attr
    , int to_set
    ) -> void
  {
    if (to_set & FUSE_SET_ATTR_MODE)
    {
      _stat.st_mode = attr->st_mode;
    }
    if (to_set & FUSE_SET_ATTR_UID)
    {
      _stat.st_uid = attr->st_uid;
    }
    if (to_set & FUSE_SET_ATTR_GID)
    {
      _stat.st_gid = attr->st_gid;
    }
    if (to_set & FUSE_SET_ATTR_ATIME)
    {
      _stat.st_atime = attr->st_atim.tv_sec;
    }
    if (to_set & FUSE_SET_ATTR_MTIME)
    {
      _stat.st_mtime = attr->st_mtim.tv_sec;
    }

    return reply::attr (request, std::addressof (_stat));
  }

  auto Common::setxattr
    ( state::writing auto const&
    , char const* key
    , std::span<char const> value
    , int flags
    ) -> int
  {
    auto make_vector
      { [&]
        {
          return std::vector<char> {std::begin (value), std::end (value)};
        }
      };

    if ( auto const entry {_xattr.find (key)}
       ; entry != std::end (_xattr)
       )
    {
      if (flags & XATTR_CREATE)
      {
        return EEXIST;
      }

      entry->second = make_vector();
    }
    else
    {
      if (flags & XATTR_REPLACE)
      {
        return ENODATA;
      }

      _xattr_keys_size
        += _xattr.emplace (key, make_vector()).first->first.size()
        ;
    }

    return 0;
  }
  auto Common::setxattr
    ( state::writing auto const& write_access
    , char const* key
    , std::span<std::byte const> value
    , int flags
    ) -> int
  {
    return setxattr
      ( write_access
      , key
      , std::span<char const>
        { mcs::util::cast<char const*> (value.data())
        , value.size()
        }
      , flags
      );
  }
  template<serialization::is_serializable T>
    auto Common::setxattr
      ( state::writing auto const& write_access
      , char const* key
      , T const& value
      , int flags
      ) -> int
  {
    auto const serialized {serialization::OArchive {value}.bytes()};

    return setxattr (write_access, key, std::span {serialized}, flags);
  }

  auto Common::getxattr
    ( state::reading auto const&
    , char const* key
    ) const -> std::optional<std::span<char const>>
  {
    if ( auto const entry {_xattr.find (key)}
       ; entry != std::end (_xattr)
       )
    {
      return std::span {entry->second};
    }

    return {};
  }

  constexpr auto Common::listxattr_size
    ( state::reading auto const&
    ) const noexcept -> size_t
  {
    // Each xattr key returned by listxattr ends with a null byte.
    // Therefore, we add the number of xattr to the size of xattr keys.
    return _xattr_keys_size + _xattr.size();
  }

  auto Common::listxattr
    ( state::reading auto const& access_token
    ) const -> std::vector<char>
  {
    auto keys {std::vector<char>{}};
    keys.reserve (listxattr_size (access_token));

    std::ranges::for_each
      ( _xattr | std::views::keys
      , [&] (auto const& key)
        {
          // The reserved size of keys using the returned value of
          // listxattr_size already takes into account the null bytes
          // placed at the end of each key.
          std::ranges::copy (key, std::back_inserter (keys));
          keys.emplace_back ('\0');
        }
      );

    return keys;
  }

  auto Common::removexattr
    ( state::writing auto const&
    , char const* key
    ) -> int
  {
    if ( auto const entry {_xattr.find (key)}
       ; entry != std::end (_xattr)
       )
    {
      _xattr_keys_size -= entry->first.size();

      _xattr.erase (entry);

      return 0;
    }

    return ENODATA;
  }

  auto Common::cleanup (state::writing auto const&) -> void
  {
    return;
  }
}
