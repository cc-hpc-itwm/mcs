// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Allow the Tuple-ish fmt, read, serialization to access the private
// part of a class.
//
#define MCS_UTIL_TUPLISH_ACCESS()                        \
  MCS_UTIL_TUPLISH_ACCESS_IMPL()

#include "detail/access.ipp"
