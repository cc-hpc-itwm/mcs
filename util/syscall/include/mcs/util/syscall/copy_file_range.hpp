// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <unistd.h>

namespace mcs::util::syscall
{
  auto copy_file_range
    ( int fd_in
    , off_t* off_in
    , int fd_out
    , off_t* off_out
    , size_t len
    , unsigned int flags
    ) -> ssize_t
    ;
}
