// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/core/storage/implementation/Files.hpp>
#include <mcs/testing/core/random/storage/MaxSize.hpp>
#include <mcs/testing/core/random/storage/implementation/Files/Prefix.hpp>

namespace mcs::testing::random
{
  template<>
    struct value<core::storage::implementation::Files::Parameter::Create>
  {
    auto operator()
      (
      ) -> core::storage::implementation::Files::Parameter::Create
      ;

  private:
    value<core::storage::implementation::Files::Prefix> _prefix;
    value<core::storage::MaxSize> _max_size;
  };
}
