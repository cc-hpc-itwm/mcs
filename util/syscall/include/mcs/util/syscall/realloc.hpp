// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <stdlib.h>

namespace mcs::util::syscall
{
  auto realloc (void*, size_t) -> void*;
}
