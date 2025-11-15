// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/deserialized_from_serialized_is_identity.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/type/ID.hpp>
#include <mcs/util/type/List.hpp>

namespace mcs::util::type
{
  TEST (UtilTypeID, ids_of_different_types_are_different)
  {
    struct A{};
    using Ts = List<int, A>;

    auto const id_i {Ts::id<int>()};
    auto const id_A {Ts::id<A>()};

    ASSERT_NE (id_i, id_A);
  }

  TEST (UtilTypeID, id_read_of_fmt_is_identity)
  {
    struct A{};
    using Ts = List<int, A>;

    testing::read_of_fmt_is_identity (Ts::id<int>());
    testing::read_of_fmt_is_identity (Ts::id<A>());
  }

  TEST (UtilTypeID, id_is_serializable)
  {
    struct A{};
    using Ts = List<int, A>;

    testing::deserialized_from_serialized_is_identity (Ts::id<int>());
    testing::deserialized_from_serialized_is_identity (Ts::id<A>());
  }
}
