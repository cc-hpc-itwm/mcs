// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/core/storage/segment/ID.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::core::control::command::segment
{
  struct Create
  {
    using Response = mcs::core::storage::segment::ID;

    mcs::core::storage::ID storage_id;
    mcs::core::storage::Parameter storage_parameter;
    mcs::core::memory::Size memory_size;
  };

  [[nodiscard]] auto make_create
    ( mcs::core::storage::ID
    , mcs::core::storage::Parameter
    , mcs::core::memory::Size
    ) -> Create
    ;
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::control::command::segment::Create>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::control::command::segment::Create const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::control::command::segment::Create>
  {
    using Type = mcs::core::control::command::segment::Create;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::control::command::segment::Create>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::control::command::segment::Create
        ;
  };
}

#include "detail/Create.ipp"
