// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

extern "C"
{
  extern int dltest_value;
  auto dltest_set (int) -> int;
  auto dltest_get() -> int;
}
