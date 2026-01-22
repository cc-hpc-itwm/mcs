// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <fmt/base.h>
#include <mcs/serialization/Concepts.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <vector>

namespace mcs::core::transport::implementation::libfabric::libfabric
{
  struct Name
  {
    [[nodiscard]] Name (std::vector<std::byte>) noexcept;

    [[nodiscard]] auto value() const noexcept -> std::vector<std::byte> const&;
    [[nodiscard]] operator std::vector<std::byte> const&() const noexcept;

  private:
    template<typename> friend struct serialization::Implementation;
    std::vector<std::byte> _value;
  };
}

namespace fmt
{
  template<>
    struct formatter<mcs::core::transport::implementation::libfabric::libfabric::Name>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( mcs::core::transport::implementation::libfabric::libfabric::Name const&
        , FormatContext& ctx
        ) const -> decltype (ctx.out());
  }
  ;
}

namespace mcs::util::read
{
  template<>
    struct Read<core::transport::implementation::libfabric::libfabric::Name>
  {
    template<typename Char>
      static auto read
        ( State<Char>&
        ) -> core::transport::implementation::libfabric::libfabric::Name
        ;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::transport::implementation::libfabric::libfabric::Name>
  {
    using Type = core::transport::implementation::libfabric::libfabric::Name;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}

#include "detail/Name.ipp"
