// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <unistd.h>

namespace mcs::util::syscall
{
  // Tries to use copy_file_range and if that returns an EXDEV
  // (invalid cross-device link), then falls back to sendfile.
  //
  auto copy_file_range_with_fallback_to_sendfile
    ( int fd_in
    , int fd_out
    , size_t len
    ) -> ssize_t
    ;
}
