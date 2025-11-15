// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/core/memory/Size.hpp>
#include <mcs/core/transport/Address.hpp>
#include <mcs/serialization/declare.hpp>
#include <variant>

namespace mcs::core::transport::implementation::ASIO::command
{
  struct Put
  {
    using Response = core::memory::Size;
    using Bytes = std::span<std::byte const>;

    Put (core::transport::Address, Bytes);
    Put (core::transport::Address, std::size_t);

    core::transport::Address destination;
    std::variant<Bytes, std::size_t> bytes_or_size;
  };
}

namespace mcs::serialization
{
  template<>
    MCS_SERIALIZATION_DECLARE_NONINTRUSIVE_IMPLEMENTATION
      (core::transport::implementation::ASIO::command::Put)
    ;
}
