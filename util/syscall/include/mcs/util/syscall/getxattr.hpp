// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/xattr.h>

namespace mcs::util::syscall
{
  auto getxattr ( char const* path
                , char const* name
                , void*
                , size_t
                ) -> ssize_t
    ;
}
