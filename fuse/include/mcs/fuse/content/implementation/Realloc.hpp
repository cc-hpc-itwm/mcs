// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/fuse/Content.hpp>

namespace mcs::fuse::content::implementation
{
  struct Realloc
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

    Realloc
      ( mcs::util::not_null<State>
      , mcs::util::not_null<state::inode::Common>
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

    auto cleanup ( state::writing auto const&) -> void;

    Realloc (Realloc const&) = delete;
    Realloc (Realloc&&) = delete;
    auto operator= (Realloc const&) -> Realloc& = delete;
    auto operator= (Realloc&&) -> Realloc& = delete;
    ~Realloc();

  private:
    struct Capacity
    {
      constexpr static auto const _xattr_key
        { "user.mcs.content.Realloc.capacity"
        };

      [[nodiscard]] Capacity
        ( mcs::util::not_null<state::inode::Common>
        , state::writing auto const&
        );

      auto set (state::writing auto const&, size_t) -> void;

      [[nodiscard]] constexpr operator size_t() const noexcept
      {
        return _value;
      }

    private:
      auto set (state::writing auto const&, size_t, int flags) -> void;

      mcs::util::not_null<state::inode::Common> _inode;
      size_t _value {0UL};
    };

    std::byte* _data {nullptr};
    size_t _size {0UL};
    Capacity _capacity;

    [[nodiscard]] static constexpr auto grow
      ( std::size_t
      ) noexcept -> std::size_t
      ;
    [[nodiscard]] static constexpr auto shrink
      ( std::size_t
      ) noexcept -> std::size_t
      ;
  };
  static_assert (is_content<Realloc>);
}

#include "detail/Realloc.ipp"
