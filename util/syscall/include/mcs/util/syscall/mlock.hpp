// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>

namespace mcs::util::syscall
{
  auto mlock (void const* addr, size_t length) -> void;
}
