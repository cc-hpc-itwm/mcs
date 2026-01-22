// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/fuse/low_level.hpp>
#include <mcs/fuse/state/Access.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <optional>
#include <span>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace mcs::fuse::state::inode
{
  struct Common
  {
    Common (::fuse_ino_t, uid_t, gid_t, nlink_t, mode_t) noexcept;

    [[nodiscard]] constexpr auto ino() const noexcept -> ::fuse_ino_t;

    [[nodiscard]] constexpr auto stat
      ( state::reading auto const&
      ) const noexcept -> struct stat
      ;

    [[nodiscard]] constexpr auto fuse_entry_param
      ( state::reading auto const&
      , double timeout_attr = 0.0
      , double timeout_entry = 0.0
      ) const noexcept -> ::fuse_entry_param
      ;

    constexpr auto inc_lookup
      ( state::writing auto const&
      ) noexcept -> void
      ;

    [[nodiscard]] constexpr auto dec_lookup
      ( state::writing auto const&
      , uint64_t count
      ) noexcept -> uint64_t
      ;

    constexpr auto inc_nlink (state::writing auto const&) noexcept -> void;
    constexpr auto dec_nlink (state::writing auto const&) noexcept -> void;

    // Pre: ! (to_set & FUSE_SET_ATTR_SIZE)
    //
    constexpr auto setattr_and_reply
      ( state::writing auto const&
      , ::fuse_req_t
      , struct stat* attr
      , int to_set
      ) -> void
      ;

    // Extended attributes
    //
    using Xattr = std::unordered_map<std::string, std::vector<char>>;

    auto setxattr
      ( state::writing auto const&
      , char const* key
      , std::span<char const> value
      , int flags
      ) -> int
      ;
    auto setxattr
      ( state::writing auto const&
      , char const* key
      , std::span<std::byte const> value
      , int flags
      ) -> int
      ;
    template<serialization::is_serializable T>
      auto setxattr
        ( state::writing auto const&
        , char const* key
        , T const& value
        , int flags
        ) -> int
      ;
    [[nodiscard]] auto getxattr
      ( state::reading auto const&
      , char const* key
      ) const -> std::optional<std::span<char const>>
      ;
    [[nodiscard]] constexpr auto listxattr_size
      ( state::reading auto const&
      ) const noexcept -> size_t
      ;
    auto listxattr
      ( state::reading auto const& access_token
      ) const -> std::vector<char>
      ;
    auto removexattr
      ( state::writing auto const&
      , char const* key
      ) -> int
      ;

    // Called immediately before destruction
    //
    auto cleanup (state::writing auto const&) -> void;

    // Never copy or move inodes.
    //
    Common (Common const&) = delete;
    Common (Common&&) = delete;
    auto operator= (Common const&) -> Common& = delete;
    auto operator= (Common&&) -> Common& = delete;
    ~Common() noexcept = default;

  private:
    uint64_t _nlookup {1U};
    size_t _xattr_keys_size {0UL};
    Xattr _xattr{};
  protected:
    struct stat _stat;
  };
}

#include "detail/Common.ipp"
