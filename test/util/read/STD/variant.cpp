// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/STD/string.hpp>
#include <mcs/testing/random/value/UTIL/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/FMT/STD/variant.hpp>
#include <mcs/util/read/STD/variant.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/string.hpp>
#include <string>

namespace mcs::util::read
{
  struct ReadOfFmtIsIdentity : public testing::random::Test{};

  TEST_F (ReadOfFmtIsIdentity, random_variant_int)
  {
    using Variant = std::variant<unsigned int>;

    testing::read_of_fmt_is_identity
      (Variant {testing::random::value<unsigned int>{}()});
  }

  TEST_F (ReadOfFmtIsIdentity, random_variant_int_string)
  {
    using Variant = std::variant<unsigned int, util::string>;

    testing::read_of_fmt_is_identity
      (Variant {testing::random::value<unsigned int>{}()});
    testing::read_of_fmt_is_identity
      (Variant {testing::random::value<util::string>{}()});
  }
}
