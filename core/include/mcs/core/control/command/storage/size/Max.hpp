// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::core::control::command::storage::size
{
  struct Max
  {
    using Response = mcs::core::storage::MaxSize;

    mcs::core::storage::ID storage_id;
    mcs::core::storage::Parameter storage_parameter;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::control::command::storage::size::Max>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::control::command::storage::size::Max const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::control::command::storage::size::Max>
  {
    using Type = mcs::core::control::command::storage::size::Max;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::control::command::storage::size::Max>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::control::command::storage::size::Max
        ;
  };
}

#include "detail/Max.ipp"
