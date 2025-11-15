// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/serialization/declare.hpp>
#include <mcs/util/tuplish/declare.hpp>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>

namespace mcs::core::storage::implementation
{
  struct Files
  {
    struct Prefix
    {
      std::filesystem::path value;
    };

    struct Tag{};

    struct Parameter
    {
      struct Create
      {
        // Requires: prefix exists
        // Requires: prefix is not mounted via NFS
        // Requires: All files in prefix must be segments, in particular
        // their names must be a valid segment_id.
        //
        // Recovers all existing segments.
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
        struct OnRemove
        {
          struct Keep{};
          struct Remove{};
        };
        using Persistency = std::variant<OnRemove::Keep, OnRemove::Remove>;

        struct Create
        {
          Persistency persistency {OnRemove::Remove{}};
        };

        struct ForceRemoval{};

        struct Remove
        {
          std::optional<ForceRemoval> force_removal{};
        };
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
      struct Create : public mcs::Error
      {
        auto parameter() const -> Parameter::Create;

        MCS_ERROR_COPY_MOVE_DEFAULT (Create);

        struct PrefixDoesNotExist : public mcs::Error
        {
          auto prefix() const -> Prefix;

          MCS_ERROR_COPY_MOVE_DEFAULT (PrefixDoesNotExist);

        private:
          friend struct Files;

          PrefixDoesNotExist (Prefix);

          Prefix _prefix;
        };

        struct UnsupportedNFSMount : public mcs::Error
        {
          auto prefix() const -> Prefix;

          MCS_ERROR_COPY_MOVE_DEFAULT (UnsupportedNFSMount);

        private:
          friend struct Files;

          UnsupportedNFSMount (Prefix);

          Prefix _prefix;
        };

        struct PrefixContainsNonSegmentFile : public mcs::Error
        {
          auto prefix() const -> Prefix;
          auto non_segment_file() const -> std::filesystem::path;

          MCS_ERROR_COPY_MOVE_DEFAULT (PrefixContainsNonSegmentFile);

        private:
          friend struct Files;

          PrefixContainsNonSegmentFile (Prefix, std::filesystem::path);

          Prefix _prefix;
          std::filesystem::path _non_segment_file;
        };

      private:
        friend struct Files;

        Create (Parameter::Create);

        Parameter::Create _parameter_create;
      };

      struct BadAlloc : public mcs::Error
      {
        constexpr auto requested() const noexcept -> memory::Size;
        constexpr auto used() const noexcept -> memory::Size;
        constexpr auto max() const noexcept -> MaxSize;

        MCS_ERROR_COPY_MOVE_DEFAULT (BadAlloc);

      private:
        friend struct Files;

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
          friend struct Files;

          UnknownSegmentID() noexcept;
        };

      private:
        friend struct Files;

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

    template<chunk::is_access Access>
      struct OpenFileImpl
    {
      struct Error
      {
        struct CouldNotOpen : public mcs::Error
        {
          CouldNotOpen (std::filesystem::path, char const*, int);

          auto path() const -> std::filesystem::path;
          auto mode() const noexcept -> char const*;
          auto error_code() const noexcept -> int;

          MCS_ERROR_COPY_MOVE_DEFAULT (CouldNotOpen);

        private:
          std::filesystem::path _path;
          std::string _mode;
          int _error_code;
        };

        struct CouldNotCloseAfterTouch : public mcs::Error
        {
          CouldNotCloseAfterTouch (std::filesystem::path, int);

          auto path() const -> std::filesystem::path;
          auto error_code() const noexcept -> int;

          MCS_ERROR_COPY_MOVE_DEFAULT (CouldNotCloseAfterTouch);

        private:
          std::filesystem::path _path;
          int _error_code;
        };
      };

      auto data() const -> typename Access::template Span<std::byte>;

    private:
      explicit OpenFileImpl
        ( std::filesystem::path
        , Parameter::Segment::Persistency
        , memory::Size
        );

      friend struct Files;

      auto remove_on_destruction() noexcept -> void;

      std::filesystem::path _path;
      Parameter::Segment::Persistency _persistency;
      std::size_t _size;
      void* _data;

      struct Deleter
      {
        auto operator() (OpenFileImpl*) const noexcept -> void;
      };

      auto fopen (char const*) const -> std::FILE*;
      auto touch() const -> void;
    };

    template<chunk::is_access Access>
      using OpenFile = std::unique_ptr
                     < OpenFileImpl<Access>
                     , typename OpenFileImpl<Access>::Deleter
                     >
      ;

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
          OpenFile<Access> _open_file;
          typename Access::template Span<std::byte> _bytes;
        };

        std::filesystem::path path;
        memory::Size file_size;
        memory::Range range;
      };
    };

    explicit Files (Parameter::Create);

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
    auto segment_use
      ( Parameter::Segment::Create
      , segment::ID
      , memory::Size
      ) -> void
      ;

    segment::ID _next_segment_id{};
    Prefix _prefix;
    MaxSize _max_size;
    memory::Size _size_used {memory::make_size (0)};
    std::unordered_map< segment::ID
                      , OpenFile<chunk::access::Mutable>
                      > _file_by_id
      ;

    auto filename (segment::ID) const -> std::filesystem::path;
  };
}

namespace mcs::serialization
{
  template<core::chunk::is_access Access>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      ( core::storage::implementation::Files::Chunk::Description<Access>
      );
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    MCS_UTIL_FMT_DECLARE
      ( mcs::core::storage::implementation::Files::Chunk::Description<Access>
      );
}

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Tag
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Prefix
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Create
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Size::Max
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Size::Used
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Segment::Create
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Segment::Remove
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::Chunk::Description
  );

MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::File::Read
  );
MCS_UTIL_TUPLISH_DECLARE_FMT_READ_SERIALIZATION
  ( mcs::core::storage::implementation::Files::Parameter::File::Write
  );

#include "detail/Files.ipp"
