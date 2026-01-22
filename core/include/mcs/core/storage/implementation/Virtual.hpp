// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <filesystem>
#include <fmt/base.h>
#include <mcs/Error.hpp>
#include <mcs/core/chunk/Access.hpp>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/memory/Range.hpp>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/implementation/Import_C_API.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/DLHandle.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::core::storage::implementation
{
  // A storage implementation that imports a c implementation from a
  // shared object during execution time.
  //
  struct Virtual
  {
    using StorageParameter = Import_C_API::StorageParameter;

    struct Tag{};

    struct Parameter
    {
      struct Create
      {
        std::filesystem::path _shared_object;
        StorageParameter _parameter_create;
      };

      using Size = Import_C_API::Parameter::Size;
      using Segment = Import_C_API::Parameter::Segment;
      using Chunk = Import_C_API::Parameter::Chunk;
      using File = Import_C_API::Parameter::File;
    };

    struct Error
    {
      using MethodNotProvided = Import_C_API::Error::MethodNotProvided;
      using Implementation = Import_C_API::Error::Implementation;

      struct Create : public mcs::Error
      {
        [[nodiscard]] constexpr auto parameter
          (
          ) const noexcept -> Parameter::Create const&
          ;

        ~Create() override;
        Create (Create const&) = default;
        Create (Create&&) noexcept = default;
        auto operator= (Create const&) -> Create& = default;
        auto operator= (Create&&) noexcept  -> Create& = default;

      private:
        friend struct Virtual;
        Create (Parameter::Create);
        Parameter::Create _parameter;
      };

      using Size = Import_C_API::Error::Size;
      using Segment = Import_C_API::Error::Segment;
      using Chunk = Import_C_API::Error::Chunk;
      using File = Import_C_API::Error::File;

      using BadAlloc = Import_C_API::Error::BadAlloc;
    };

    using Chunk = Import_C_API::Chunk;

    explicit Virtual (Parameter::Create);

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
    util::DLHandle _dlhandle;
    Import_C_API _imported_c_api;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Virtual::Tag>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Virtual::Tag const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Virtual::Tag>
  {
    using Type = mcs::core::storage::implementation::Virtual::Tag;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Virtual::Tag>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Virtual::Tag
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::storage::implementation::Virtual::Parameter::Create>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::storage::implementation::Virtual::Parameter::Create const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::storage::implementation::Virtual::Parameter::Create>
  {
    using Type = mcs::core::storage::implementation::Virtual::Parameter::Create;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::storage::implementation::Virtual::Parameter::Create>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::storage::implementation::Virtual::Parameter::Create
        ;
  };
}

#include "detail/Virtual.ipp"
