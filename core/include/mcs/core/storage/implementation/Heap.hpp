// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/access.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/Buffer.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/tuplish/declare.hpp>
#include <memory>
#include <optional>
#include <span>
#include <unordered_map>

namespace mcs::core::storage::implementation
{
  struct Heap
  {
    struct Tag{};

    struct Parameter
    {
      struct Create
      {
        MaxSize max_size;
      };

      struct Size
      {
        struct Max{};
        struct Used{};
      };

      struct Segment
      {
        struct MLOCKed{};

        struct Create
        {
          std::optional<MLOCKed> mlocked{};
        };

        struct Remove{};
      };

      struct Chunk
      {
        struct Description{};
      };

      struct File
      {
        struct Read{};
        struct Write{};
      };
    };

    struct Error
    {
      struct BadAlloc : public mcs::Error
      {
        constexpr auto requested() const noexcept -> memory::Size;
        constexpr auto used() const noexcept -> memory::Size;
        constexpr auto max() const noexcept -> MaxSize;

        MCS_ERROR_COPY_MOVE_DEFAULT (BadAlloc);

      private:
        friend struct Heap;

        BadAlloc
          ( memory::Size requested
          , memory::Size used
          , MaxSize max
          ) noexcept;

        memory::Size _requested;
        memory::Size _used;
        MaxSize _max;
      };

      struct ChunkDescription : public mcs::Error
      {
        constexpr auto parameter
          (
          ) const noexcept -> Parameter::Chunk::Description const&
          ;
        constexpr auto segment_id() const noexcept -> segment::ID;
        constexpr auto memory_range() const noexcept -> memory::Range;

        MCS_ERROR_COPY_MOVE_DEFAULT (ChunkDescription);

        struct UnknownSegmentID : public mcs::Error
        {
          MCS_ERROR_COPY_MOVE_DEFAULT (UnknownSegmentID);

        private:
          friend struct Heap;

          UnknownSegmentID() noexcept;
        };

      private:
        friend struct Heap;

        ChunkDescription
          ( Parameter::Chunk::Description
          , segment::ID
          , memory::Range
          ) noexcept
          ;

        Parameter::Chunk::Description _parameter;
        segment::ID _segment_id;
        memory::Range _memory_range;
      };
    };

    struct Chunk
    {
      template<chunk::is_access Access>
        struct Description
      {
        struct State
        {
          explicit State (Description<Access> const&);

          auto bytes() const -> typename Access::template Span<std::byte>;

        private:
          typename Access::template Span<std::byte> _bytes;
        };

        constexpr Description
          ( std::uintmax_t
          , std::size_t
          , memory::Range
          ) noexcept
          ;

      private:
        std::uintmax_t _begin;
        std::size_t _size;
        memory::Range _range;

        MCS_SERIALIZATION_ACCESS();
        MCS_UTIL_FMT_ACCESS();

        template<chunk::is_access Access_>
          friend constexpr auto operator==
            ( Description<Access_> const&
            , Description<Access_> const&
            ) noexcept -> bool
          ;
      };
    };

    explicit Heap (Parameter::Create) noexcept;

    auto size_max
      ( Parameter::Size::Max
      ) const -> MaxSize
      ;
    auto size_used
      ( Parameter::Size::Used
      ) const -> memory::Size
      ;

    auto segment_create
      ( Parameter::Segment::Create
      , memory::Size
      ) -> segment::ID
      ;

    auto segment_remove
      ( Parameter::Segment::Remove
      , segment::ID
      ) -> memory::Size
      ;

    template<chunk::is_access Access>
      auto chunk_description
        ( Parameter::Chunk::Description
        , segment::ID
        , memory::Range
        ) const -> Chunk::Description<Access>
      ;

    auto file_read
      ( Parameter::File::Read
      , segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;
    auto file_write
      ( Parameter::File::Write
      , segment::ID
      , memory::Offset
      , std::filesystem::path
      , memory::Range
      ) const -> memory::Size
      ;

  private:
    segment::ID _next_segment_id{};
    MaxSize _max_size;
    memory::Size _size_used {memory::make_size (0)};
    struct Deleter
    {
      auto operator() (std::byte*) const -> void;
      std::optional<std::size_t> _unlock{};
    };
    using Buffer = util::Buffer<std::byte[], Deleter>;
    std::unordered_map<segment::ID, Buffer> _buffer_by_id;
  };
}


namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::implementation::Heap::Chunk::Description<Access>
      );
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::Heap::Chunk::Description<Access>
      );
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Tag
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Create
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Size::Used
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Segment::MLOCKed
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Segment::Create
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Segment::Remove
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::Chunk::Description
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Heap::Parameter::File::Write
  );

#include "detail/Heap.ipp"
