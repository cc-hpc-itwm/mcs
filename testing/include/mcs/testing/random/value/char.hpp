// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  // produces random characters
  //
  template<> struct value<char> : public value<std::int_least8_t>
  {
    using Base = value<std::int_least8_t>;

    using Base::Min;
    using Base::Max;
    using Base::Base;

    auto operator()() -> char
    {
      return Base::operator()();
    }
  };
}
