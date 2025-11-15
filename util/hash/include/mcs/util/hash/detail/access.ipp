// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>

#define MCS_UTIL_HASH_ACCESS_IMPL()                      \
  template<typename> friend struct std::hash
