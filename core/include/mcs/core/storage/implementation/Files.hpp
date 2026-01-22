// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/serialization/STD/filesystem/path.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>
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

        ~Create() override;
        Create (Create const&) = default;
        Create (Create&&) noexcept = default;
        auto operator= (Create const&) -> Create& = default;
        auto operator= (Create&&) noexcept  -> Create& = default;

        struct PrefixDoesNotExist : public mcs::Error
        {
          auto prefix() const -> Prefix;

          ~PrefixDoesNotExist() override;
          PrefixDoesNotExist (PrefixDoesNotExist const&) = default;
          PrefixDoesNotExist (PrefixDoesNotExist&&) noexcept = default;
          auto operator= (PrefixDoesNotExist const&) -> PrefixDoesNotExist& = default;
          auto operator= (PrefixDoesNotExist&&) noexcept  -> PrefixDoesNotExist& = default;

        private:
          friend struct Files;

          PrefixDoesNotExist (Prefix);

          Prefix _prefix;
        };

        struct UnsupportedNFSMount : public mcs::Error
        {
          auto prefix() const -> Prefix;

          ~UnsupportedNFSMount() override;
          UnsupportedNFSMount (UnsupportedNFSMount const&) = default;
          UnsupportedNFSMount (UnsupportedNFSMount&&) noexcept = default;
          auto operator= (UnsupportedNFSMount const&) -> UnsupportedNFSMount& = default;
          auto operator= (UnsupportedNFSMount&&) noexcept  -> UnsupportedNFSMount& = default;

        private:
          friend struct Files;

          UnsupportedNFSMount (Prefix);

          Prefix _prefix;
        };

        struct PrefixContainsNonSegmentFile : public mcs::Error
        {
          auto prefix() const -> Prefix;
          auto non_segment_file() const -> std::filesystem::path;

          ~PrefixContainsNonSegmentFile() override;
          PrefixContainsNonSegmentFile (PrefixContainsNonSegmentFile const&) = default;
          PrefixContainsNonSegmentFile (PrefixContainsNonSegmentFile&&) noexcept = default;
          auto operator= (PrefixContainsNonSegmentFile const&) -> PrefixContainsNonSegmentFile& = default;
          auto operator= (PrefixContainsNonSegmentFile&&) noexcept  -> PrefixContainsNonSegmentFile& = default;

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

        ~BadAlloc() override;
        BadAlloc (BadAlloc const&) = default;
        BadAlloc (BadAlloc&&) noexcept = default;
        auto operator= (BadAlloc const&) -> BadAlloc& = default;
        auto operator= (BadAlloc&&) noexcept  -> BadAlloc& = default;

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

        ~ChunkDescription() override;
        ChunkDescription (ChunkDescription const&) = default;
        ChunkDescription (ChunkDescription&&) noexcept = default;
        auto operator= (ChunkDescription const&) -> ChunkDescription& = default;
        auto operator= (ChunkDescription&&) noexcept  -> ChunkDescription& = default;

        struct UnknownSegmentID : public mcs::Error
        {
          ~UnknownSegmentID() override;
          UnknownSegmentID (UnknownSegmentID const&) = default;
          UnknownSegmentID (UnknownSegmentID&&) noexcept = default;
          auto operator= (UnknownSegmentID const&) -> UnknownSegmentID& = default;
          auto operator= (UnknownSegmentID&&) noexcept  -> UnknownSegmentID& = default;

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

          ~CouldNotOpen() override;
          CouldNotOpen (CouldNotOpen const&) = default;
          CouldNotOpen (CouldNotOpen&&) noexcept = default;
          auto operator= (CouldNotOpen const&) -> CouldNotOpen& = default;
          auto operator= (CouldNotOpen&&) noexcept  -> CouldNotOpen& = default;

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

          ~CouldNotCloseAfterTouch() override;
          CouldNotCloseAfterTouch (CouldNotCloseAfterTouch const&) = default;
          CouldNotCloseAfterTouch (CouldNotCloseAfterTouch&&) noexcept = default;
          auto operator= (CouldNotCloseAfterTouch const&) -> CouldNotCloseAfterTouch& = default;
          auto operator= (CouldNotCloseAfterTouch&&) noexcept  -> CouldNotCloseAfterTouch& = default;

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
    struct Implementation<core::storage::implementation::Files::Chunk::Description<Access>>
  {
    using Type = core::storage::implementation::Files::Chunk::Description<Access>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    struct formatter<mcs::core::storage::implementation::Files::Chunk::Description<Access>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Chunk::Description<Access> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Tag>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Tag const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Tag>
  {
    using Type = mcs::core::storage::implementation::Files::Tag;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Tag>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Tag
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Prefix>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Prefix const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Prefix>
  {
    using Type = mcs::core::storage::implementation::Files::Prefix;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Prefix>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Prefix
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Create>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Create const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Create>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Create;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Create>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Create
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Size::Max>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Size::Max const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Size::Max>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Size::Max;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Size::Max>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Size::Max
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Size::Used>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Size::Used const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Size::Used>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Size::Used;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Size::Used>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Size::Used
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Keep
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Segment::OnRemove::Remove
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Segment::Create>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Segment::Create const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Segment::Create>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Segment::Create;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Segment::Create>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Segment::Create
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Segment::ForceRemoval
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Segment::Remove const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Segment::Remove;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Segment::Remove>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Segment::Remove
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::Chunk::Description const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::Chunk::Description;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::Chunk::Description>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::Chunk::Description
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::File::Read>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::File::Read const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::File::Read>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::File::Read;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::File::Read>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::File::Read
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Files::Parameter::File::Write>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Files::Parameter::File::Write const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Files::Parameter::File::Write>
  {
    using Type = mcs::core::storage::implementation::Files::Parameter::File::Write;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Files::Parameter::File::Write>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Files::Parameter::File::Write
        ;
  };
}

#include "detail/Files.ipp"
