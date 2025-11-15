// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/util/string.hpp>

namespace mcs::testing::random
{
  // produces random util::strings
  //
  template<> struct value<util::string>
  {
    // Arguments for the random_string generator
    //
    template<typename... Args> explicit value (Args&&...);

    auto operator()() -> util::string;

  private:
    value<std::string> _random_string;
  };
}

#include "detail/string.ipp"
