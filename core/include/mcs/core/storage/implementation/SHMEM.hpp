// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <fcntl.h>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/Buffer.hpp>
#include <mcs/util/string.hpp>
#include <mcs/util/tuplish/declare.hpp>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <sys/stat.h>
#include <tuple>
#include <unordered_map>

namespace mcs::core::storage::implementation
{
  struct SHMEM
  {
    struct Prefix
    {
      util::string value;
    };

    struct Tag{};

    struct Parameter
    {
      struct Create
      {
        // The shared memory segments will consist of this prefix
        // followed by the segment id.
        //
        Prefix prefix;
        MaxSize max_size;
      };

      struct Size
      {
        struct Max{};
        struct Used{};
      };

      struct Segment
      {
        struct AccessMode
        {
          ::mode_t value;
        };

        struct MLOCKed{};

        struct Create
        {
          AccessMode access_mode {S_IRUSR | S_IWUSR};
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
        friend struct SHMEM;

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
          friend struct SHMEM;

          UnknownSegmentID() noexcept;
        };

      private:
        friend struct SHMEM;

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

    struct Unmap
    {
      auto operator() (void* ptr) const -> void;
      std::size_t _size;
      bool _unlock {false};
    };

    template<chunk::is_access Access>
      struct CacheImpl
    {
      auto data() const -> typename Access::template Span<std::byte>;

    private:
      friend struct SHMEM;

      struct Create{};
      explicit CacheImpl
        ( Create
        , Prefix
        , segment::ID
        , memory::Size
        , Parameter::Segment::AccessMode
        , std::optional<Parameter::Segment::MLOCKed>
        );

      struct Open{};
      explicit CacheImpl (Open, Prefix, segment::ID, memory::Size);

      Prefix _prefix;
      segment::ID _segment_id;
      using Name = std::string;
      auto name() const -> Name;
      std::optional<Name> _unlink{};
      util::Buffer<void, Unmap> _buffer;
      auto shm_open (Name, std::size_t) -> decltype (_buffer);
      auto shm_create
        ( Name
        , std::size_t
        , Parameter::Segment::AccessMode
        , std::optional<Parameter::Segment::MLOCKed>
        ) -> decltype (_buffer);

      struct Deleter
      {
        auto operator() (CacheImpl*) const noexcept -> void;
      };
    };

    template<chunk::is_access Access>
      using Cache = std::unique_ptr
                  < CacheImpl<Access>
                  , typename CacheImpl<Access>::Deleter
                  >;

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
          Cache<Access> _cache;
          typename Access::template Span<std::byte> _bytes;
        };

        Prefix prefix;
        segment::ID segment_id;
        memory::Size size;
        memory::Range range;
      };
    };

    explicit SHMEM (Parameter::Create) noexcept;

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
    Prefix _prefix;
    segment::ID _next_segment_id{};
    MaxSize _max_size;
    memory::Size _size_used {memory::make_size (0)};
    std::unordered_map< segment::ID
                      , Cache<chunk::access::Mutable>
                      > _caches_by_id;
  };
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::implementation::SHMEM::Chunk::Description<Access>
      );
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::SHMEM::Chunk::Description<Access>
      );
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Tag
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Prefix
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Create
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Size::Used
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::SHMEM::Parameter::File::Write
  );

#include "detail/SHMEM.ipp"
