// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/serialization/Concepts.hpp>
#include <memory>
#include <span>
#include <variant>

namespace mcs::core::transport::implementation::ASIO::command
{
  struct Put
  {
    using Response = core::memory::Size;
    struct Source
    {
      [[nodiscard]] virtual auto data
        (
        ) const -> std::span<std::byte const> = 0
        ;
      virtual ~Source() = default;
    };

    Put (core::transport::Address, std::unique_ptr<Source>);
    Put (core::transport::Address, std::size_t);

    core::transport::Address destination;
    std::variant<std::unique_ptr<Source>, std::size_t> source_or_size;
  };
}

namespace mcs::serialization
{
  template<>
    struct Implementation<core::transport::implementation::ASIO::command::Put>
  {
    using Type = core::transport::implementation::ASIO::command::Put;

    static auto output (OArchive&, Type const&) -> OArchive&;
    static auto input (IArchive&) -> Type;
  };
}
