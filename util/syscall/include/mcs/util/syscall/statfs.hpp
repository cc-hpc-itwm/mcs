// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/statfs.h>

namespace mcs::util::syscall
{
  auto statfs (char const* path) -> struct statfs;
}
