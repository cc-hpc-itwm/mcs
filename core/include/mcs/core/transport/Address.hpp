// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Offset.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::core::transport
{
  struct Address
  {
    storage::ID storage_id;
    storage::Parameter storage_parameter_chunk_description;
    storage::segment::ID segment_id;
    memory::Offset offset;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::transport::Address>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::transport::Address const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::transport::Address>
  {
    using Type = mcs::core::transport::Address;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::transport::Address>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::transport::Address
        ;
  };
}

#include "detail/Address.ipp"
