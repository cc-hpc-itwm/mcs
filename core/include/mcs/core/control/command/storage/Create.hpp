// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <fmt/base.h>
#include <mcs/core/storage/Concepts.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/type/ID.hpp>

namespace mcs::core::control::command::storage
{
  template<core::storage::is_implementation... StorageImplementations>
    struct Create
  {
    using Response = core::storage::ID;

    util::type::ID<StorageImplementations...> implementation_id;
    core::storage::Parameter storage_parameter;
  };

  template< core::storage::is_implementation StorageImplementation
          , core::storage::is_implementation... StorageImplementations
          >
    auto create
      ( typename StorageImplementation::Parameter::Create
      ) noexcept -> Create<StorageImplementations...>
    ;
}

namespace fmt
{
  template<mcs::core::storage::is_implementation... StorageImplementations>
    struct formatter<mcs::core::control::command::storage::Create<StorageImplementations...>>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::control::command::storage::Create<StorageImplementations...> const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  };
}

namespace mcs::serialization
{
  template<core::storage::is_implementation... StorageImplementations>
    struct Implementation<core::control::command::storage::Create<StorageImplementations...>>
  {
    using Type = core::control::command::storage::Create<StorageImplementations...>;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<core::storage::is_implementation... StorageImplementations>
    struct Read<core::control::command::storage::Create<StorageImplementations...>>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::control::command::storage::Create<StorageImplementations...>
        ;
  };
}

#include "detail/Create.ipp"
