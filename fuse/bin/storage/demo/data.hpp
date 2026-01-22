// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <span>

namespace mcs::fuse::demo
{
  // Fill buffer with pattern data starting at the given offset.
  //
  auto generate (std::span<std::byte> buffer, std::size_t offset) -> void;

  // Verify buffer contains expected pattern data starting at the
  // given offset. Throws std::runtime_error on mismatch.
  //
  auto verify (std::span<std::byte const> buffer, std::size_t offset) -> void;
}
