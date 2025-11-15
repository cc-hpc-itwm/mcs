// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include "dlerror.hpp"
#include <dlfcn.h>

namespace mcs::util::syscall
{
  auto dlopen (char const* filename, int flags) -> void*;
}
