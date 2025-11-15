// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Allow std::hash to access private members.
//
#define MCS_UTIL_HASH_ACCESS()                           \
  MCS_UTIL_HASH_ACCESS_IMPL()

#include "detail/access.ipp"
