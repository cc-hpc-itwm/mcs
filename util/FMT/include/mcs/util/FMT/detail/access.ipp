// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/base.h>

#define MCS_UTIL_FMT_ACCESS_IMPL()                                      \
  template<typename, typename, typename> friend struct fmt::formatter
