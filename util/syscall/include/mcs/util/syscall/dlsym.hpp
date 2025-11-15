// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include "dlerror.hpp"
#include <dlfcn.h>

namespace mcs::util::syscall
{
  auto dlsym (void* handle, char const* symbol) -> void*;
}
