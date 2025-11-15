// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include "library.hpp"

extern "C"
{
  int dltest_value {-1};

  auto dltest_set (int value) -> int
  {
    return dltest_value = value;
  }

  auto dltest_get() -> int
  {
    return dltest_value;
  }
}
