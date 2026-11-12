// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/storage/ID.hpp>
#include <mcs/core/storage/MaxSize.hpp>
#include <mcs/core/storage/Parameter.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/require_semi.hpp>

namespace mcs::core::control::command::storage
{
  struct Size
  {
    struct Response
    {
      [[nodiscard]] constexpr Response
        ( core::storage::MaxSize
        , core::memory::Size
        ) noexcept
        ;
      [[nodiscard]] constexpr auto max
        (
        ) const noexcept -> core::storage::MaxSize
        ;
      [[nodiscard]] constexpr auto used
        (
        ) const noexcept -> core::memory::Size
        ;

    private:
      core::storage::MaxSize _max;
      core::memory::Size _used;

      friend struct fmt::formatter<Size>;
      friend struct serialization::Implementation<Response>;
      friend struct util::read::Read<Size>;
    };

    core::storage::ID storage_id;
    core::storage::Parameter storage_parameter_size_max;
    core::storage::Parameter storage_parameter_size_used;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::control::command::storage::Size::Response>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::control::command::storage::Size::Response const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::control::command::storage::Size::Response>
  {
    using Type = mcs::core::control::command::storage::Size::Response;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::control::command::storage::Size::Response>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::control::command::storage::Size::Response
        ;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::control::command::storage::Size>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::control::command::storage::Size const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<mcs::core::control::command::storage::Size>
  {
    using Type = mcs::core::control::command::storage::Size;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

namespace mcs::util::read
{
  template<>
    struct Read<mcs::core::control::command::storage::Size>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> mcs::core::control::command::storage::Size
        ;
  };
}

#include "detail/Size.ipp"
