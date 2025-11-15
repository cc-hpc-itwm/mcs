// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <filesystem>
#include <mcs/testing/core/random/storage/implementation/Virtual/Parameter/Create.hpp>

namespace mcs::testing::random
{
  using Create = core::storage::implementation::Virtual::Parameter::Create;

  auto value<Create>::operator()() -> Create
  {
    return Create
      { std::filesystem::path {_shared_object()}
      , _parameter_create()
      };
  }
}
