// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/fuse/Content.hpp>
#include <vector>

namespace mcs::fuse::content::implementation
{
  struct Vector
  {
    struct State
    {
      constexpr auto fuse_init (fuse_conn_info*) const noexcept -> void;
      constexpr auto fuse_destroy() const noexcept -> void;

      [[nodiscard]] auto f_bsize
        ( state::reading auto const&
        ) const -> unsigned long
        ;
      [[nodiscard]] auto f_frsize
        ( state::reading auto const&
        ) const -> unsigned long
        ;
      [[nodiscard]] auto f_blocks
        ( state::reading auto const&
        ) const -> fsblkcnt_t
        ;
      [[nodiscard]] auto f_bfree
        ( state::reading auto const&
        ) const -> fsblkcnt_t
        ;
      [[nodiscard]] auto f_bavail
        ( state::reading auto const&
        ) const -> fsblkcnt_t
        ;
    };

    Vector
      ( mcs::util::not_null<State>
      , util::not_null<state::inode::Common>
      , state::writing auto const&
      , ::fuse_ino_t
      )
      ;

    [[nodiscard]] constexpr auto size
      ( state::reading auto const&
      ) const noexcept -> size_t
      ;

    using Data = content::Data;
    [[nodiscard]] auto data
      ( state::reading auto const&
      , size_t
      , off_t
      ) const -> Data
      ;

    // Post: ! (size() < size)
    auto resize
      ( state::writing auto const&
      , size_t
      ) -> content::result::Resize
      ;

    [[nodiscard]] auto write
      ( state::writing auto const&
      , content::Data const&
      , off_t
      ) -> content::result::Write
      ;

    [[nodiscard]] auto fallocate
      ( state::writing auto const&
      , int mode
      , off_t offset
      , off_t size
      ) -> int
      ;

    auto cleanup (state::writing auto const&) -> void;

    Vector (Vector const&) = delete;
    Vector (Vector&&) = delete;
    auto operator= (Vector const&) -> Vector& = delete;
    auto operator= (Vector&&) -> Vector& = delete;
    ~Vector() = default;

  private:
    constexpr static auto const _xattr_key_capacity
      { "user.mcs.content.Vector.capacity"
      };
    util::not_null<state::inode::Common> _inode;

    auto xattr_set_capacity (state::writing auto const&, int flags) -> void;

    std::vector<std::byte> _data;
  };
  static_assert (is_content<Vector>);
}

#include "detail/Vector.ipp"
