// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/testing/random/value/STD/string.hpp>

namespace mcs::testing::random
{
  template<> struct value<core::storage::implementation::Files::Prefix>
  {
    auto operator()() -> core::storage::implementation::Files::Prefix;

  private:
    value<std::string> _string;
  };
}
