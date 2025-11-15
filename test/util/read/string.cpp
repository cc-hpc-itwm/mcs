// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/string.hpp>
#include <string>

namespace
{
  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

TEST_F (ReadOfFmtIsIdentityR, string)
{
  mcs::testing::read_of_fmt_is_identity
    (mcs::util::string {mcs::testing::random::value<std::string>{}()});
}
