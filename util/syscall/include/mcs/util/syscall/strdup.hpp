// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <string.h>

namespace mcs::util::syscall
{
  auto strdup (char const*) -> char*;
}
