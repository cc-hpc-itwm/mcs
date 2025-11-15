// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Allow fmt::formatter to access private members.
//
#define MCS_UTIL_FMT_ACCESS()                            \
  MCS_UTIL_FMT_ACCESS_IMPL()

#include "detail/access.ipp"
