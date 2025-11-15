// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#define MCS_UTIL_READ_ACCESS()                          \
  MCS_UTIL_READ_ACCESS_IMPL()

#include "detail/access.ipp"
