// Copyright (C) 2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstddef>
#include <cstdint>
#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  // produces random bytes
  //
  template<> struct value<std::byte> : public value<std::uint_least8_t>
  {
    using Base = value<std::uint_least8_t>;

    using Base::Min;
    using Base::Max;
    using Base::Base;

    auto operator()() -> std::byte
    {
      return std::byte {Base::operator()()};
    }
  };
}
