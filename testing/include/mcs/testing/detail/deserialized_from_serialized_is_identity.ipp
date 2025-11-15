// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/serialization/load_from.hpp>
#include <mcs/serialization/save.hpp>

namespace mcs::testing
{
  template<serialization_identity_testable T>
    constexpr auto deserialized_from_serialized_is_identity (T const& x)
  {
    ASSERT_EQ
      ( x
      , serialization::load_from<T> (serialization::OArchive {x}.bytes())
      );
  }
}
