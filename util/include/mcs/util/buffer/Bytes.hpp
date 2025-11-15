// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <mcs/util/Buffer.hpp>

namespace mcs::util::buffer
{
  struct Bytes : public Buffer<std::byte[]>
  {
    constexpr Bytes() noexcept = default;
    Bytes (std::size_t);
  };
}
