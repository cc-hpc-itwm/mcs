// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/Chunk.hpp>
#include <mcs/core/chunk/access/Const.hpp>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/fuse/Content.hpp>
#include <mcs/fuse/content/implementation/storage/Part.hpp>
#include <set>
#include <vector>

namespace mcs::fuse::content::implementation
{
  template<core::storage::is_implementation StorageImplementation>
    struct Storage
  {
    // The underlying core storage.
    //
    struct State
    {
      using Parameter = typename StorageImplementation::Parameter;

      [[nodiscard]] State
        ( typename Parameter::Create
        , typename Parameter::Size::Max
        , typename Parameter::Size::Used
        , typename Parameter::Segment::Create
        , typename Parameter::Segment::Remove
        , typename Parameter::Chunk::Description
        );

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

      [[nodiscard]] auto segment_remove
        ( state::writing auto const&
        , core::storage::segment::ID
        ) -> core::memory::Size
        ;
      [[nodiscard]] auto segment_create
        ( state::writing auto const&
        , core::memory::Size
        ) -> core::storage::segment::ID
        ;
      template<core::chunk::is_access Access>
        [[nodiscard]] auto chunk
          ( state::reading auto const&
          , core::storage::segment::ID segment_id
          , core::memory::Range range
          ) const -> core::Chunk<Access, StorageImplementation>
        ;

    private:
      StorageImplementation _storage;
      typename Parameter::Size::Max _parameter_size_max;
      typename Parameter::Size::Used _parameter_size_used;
      typename Parameter::Segment::Create _parameter_segment_create;
      typename Parameter::Segment::Remove _parameter_segment_remove;
      typename Parameter::Chunk::Description _parameter_chunk_description;
    };

    Storage
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

    // Wrapper around content::Data that keeps the core::Chunks alive.
    //
    struct Data
    {
      [[nodiscard]] constexpr operator content::Data const&() const noexcept;

      using CoreChunk
        = core::Chunk<core::chunk::access::Const, StorageImplementation>
        ;

      auto emplace (CoreChunk) -> void;

    private:
      std::vector<CoreChunk> _chunks;
      content::Data _data;
    };

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

    auto ioctl
      ( state::reading auto const&
      , ::fuse_req_t request
      , unsigned int command
      , void *arg
      , void const* in_buf
      , size_t in_buf_size
      , size_t out_buf_size
      ) const -> void
      ;

    auto cleanup (state::writing auto const&) -> void;

    Storage (Storage const&) = delete;
    Storage (Storage&&) = delete;
    auto operator= (Storage const&) -> Storage& = delete;
    auto operator= (Storage&&) -> Storage& = delete;
    ~Storage() noexcept = default;

  private:
    mcs::util::not_null<State> _state;
    constexpr static auto const _xattr_key_capacity
      { "user.mcs.inode.Storage.capacity"
      };
    util::not_null<state::inode::Common> _inode;

    auto xattr_set_capacity (state::writing auto const&, int flags) -> void;

    std::size_t _size {0UL};
    std::size_t _capacity {0UL};

    // \note vector of (end, segment_id) would be enough
    using Parts = std::vector<storage::Part>;
    Parts _parts;

    auto free_all_parts
      ( state::writing auto const&
      ) -> void
      ;
    auto free_range_of_parts
      ( state::writing auto const&
      , Parts::iterator begin
      , Parts::iterator end
      ) -> void
      ;

    auto lower_bound
      ( core::memory::Offset begin
      ) -> Parts::iterator
      ;
    auto lower_bound
      ( core::memory::Offset begin
      ) const -> Parts::const_iterator
      ;

    template<core::chunk::is_access Access>
      auto make_chunk
        ( state::reading auto const&
        , storage::Part const&
        , core::memory::Range
        ) const -> core::Chunk<Access, StorageImplementation>
      ;

    template<core::chunk::is_access Access, typename UseChunk>
      requires (std::invocable< UseChunk
                              , core::Chunk<Access, StorageImplementation>
                              >
               )
      auto for_each_chunk
        ( state::reading auto const&
        , core::memory::Range
        , UseChunk
        ) const -> void
      ;
  };
}

#include "detail/Storage.ipp"
