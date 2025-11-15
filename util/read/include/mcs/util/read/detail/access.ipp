// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>

#define MCS_UTIL_READ_ACCESS_IMPL()                     \
  template<typename> friend struct util::read::Read
