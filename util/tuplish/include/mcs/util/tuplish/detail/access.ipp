// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/serialization/access.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/read/access.hpp>

#define MCS_UTIL_TUPLISH_ACCESS_IMPL()                   \
  MCS_UTIL_FMT_ACCESS();                                 \
  MCS_SERIALIZATION_ACCESS();                            \
  MCS_UTIL_READ_ACCESS()
