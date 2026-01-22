// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/share_service/SupportedStorageImplementations.hpp>
#include <mcs/util/ASIO/Connectable.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::share_service
{
  // Self contained description of a piece of memory.
  //
  struct Chunk
  {
    util::ASIO::AnyConnectable provider_connectable;
    core::storage::ID storage_id;
    SupportedStorageImplementations::ID storage_implementation_id;
    core::storage::segment::ID segment_id;
    core::memory::Size size;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::share_service::Chunk>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::share_service::Chunk const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::share_service::Chunk>
  {
    using Type = mcs::share_service::Chunk;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::share_service::Chunk>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::share_service::Chunk
        ;
  };
}

#include "detail/Chunk.ipp"
