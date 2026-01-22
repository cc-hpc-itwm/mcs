// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <fcntl.h>
#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/Buffer.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>
#include <mcs/util/string.hpp>
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

        ~BadAlloc() override;
        BadAlloc (BadAlloc const&) = default;
        BadAlloc (BadAlloc&&) noexcept = default;
        auto operator= (BadAlloc const&) -> BadAlloc& = default;
        auto operator= (BadAlloc&&) noexcept  -> BadAlloc& = default;

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
    struct Implementation<core::storage::implementation::SHMEM::Chunk::Description<Access>>
  {
    using Type = core::storage::implementation::SHMEM::Chunk::Description<Access>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace fmt
{
  template<mcs::core::chunk::is_access Access>
    struct formatter<mcs::core::storage::implementation::SHMEM::Chunk::Description<Access>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Chunk::Description<Access> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Tag>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Tag const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Tag>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Tag;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Tag>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Tag
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Prefix>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Prefix const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Prefix>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Prefix;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Prefix>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Prefix
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Create>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Create const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Create>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Create;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Create>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Create
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Size::Max const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Size::Max;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Size::Max>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Size::Max
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Size::Used const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Size::Used;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Size::Used>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Size::Used
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::AccessMode
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::MLOCKed
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::Create
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Segment::Remove
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::Chunk::Description
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::File::Read const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::File::Read;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::File::Read>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::File::Read
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::SHMEM::Parameter::File::Write const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>
  {
    using Type = mcs::core::storage::implementation::SHMEM::Parameter::File::Write;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::SHMEM::Parameter::File::Write>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::SHMEM::Parameter::File::Write
        ;
  };
}

#include "detail/SHMEM.ipp"
