// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Virtual.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/STD/vector.hpp>
#include <mcs/testing/random/value/integral.hpp>

namespace mcs::testing::random
{
  template<>
    struct value<core::storage::implementation::Virtual::Parameter::Create>
  {
    auto operator()
      (
      ) -> core::storage::implementation::Virtual::Parameter::Create
      ;

  private:
    value<std::string> _shared_object;
    value<core::storage::implementation::Virtual::StorageParameter>
      _parameter_create;
  };
}
