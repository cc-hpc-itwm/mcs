// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/random/random_device.hpp>
#include <mcs/testing/random/seed.hpp>
#include <mcs/util/cast.hpp>

namespace mcs::testing::random
{
  auto random_device() -> std::default_random_engine&
  {
    using Engine = std::default_random_engine;
    using Seed = decltype (seed());
    using Param = Engine::result_type;
    if constexpr (sizeof (Seed) <= sizeof (Param))
    {
      static auto d {Engine {util::cast<Param> (seed())}};
      return d;
    }
    else
    {
      // seed() has too many bytes, just cast anyways, fingers crossed
      static auto d {Engine {static_cast<Param> (seed())}};
      return d;
    }
  }
}
