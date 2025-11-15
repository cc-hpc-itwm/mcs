// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/util/TemporaryFile.hpp>

namespace mcs::util
{
  TEST (TemporaryFile, is_move_constructible)
  {
    static_assert (std::is_move_constructible_v<TemporaryFile>);
  }
  TEST (TemporaryFile, is_move_assignable)
  {
    static_assert (std::is_move_assignable_v<TemporaryFile>);
  }
  TEST (TemporaryFile, is_not_copy_constructible)
  {
    static_assert (!std::is_copy_constructible_v<TemporaryFile>);
  }
  TEST (TemporaryFile, is_not_copy_assignable)
  {
    static_assert (!std::is_copy_assignable_v<TemporaryFile>);
  }
}
