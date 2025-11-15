// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/UniqID.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/CIdent.hpp>

namespace mcs::testing
{
  struct UniqIDF : public mcs::testing::random::Test{};

  TEST_F (UniqIDF, id_has_given_prefix)
  {
    auto prefix {random::value<random::CIdent>{}()};
    UniqID id {prefix};

    ASSERT_TRUE (id().starts_with (prefix));
  }
}
