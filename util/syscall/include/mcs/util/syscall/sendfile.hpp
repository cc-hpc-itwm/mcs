// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <sys/sendfile.h>

namespace mcs::util::syscall
{
  auto sendfile
    ( int out_fd
    , int in_fd
    , off_t* offset
    , size_t count
    ) -> ssize_t
    ;
}
