// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/UTIL/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/FMT/STD/optional.hpp>
#include <mcs/util/read/STD/optional.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/string.hpp>

namespace mcs::util::read
{
  TEST (ReadOfFmtIsIdentity, nullopt)
  {
    testing::read_of_fmt_is_identity (std::optional<unsigned int>{});
  }

  namespace
  {
    using Types = ::testing::Types
      < unsigned int
      , util::string
      >;
    template<class> struct ReadOfFmtIsIdentityT : public testing::random::Test{};
    TYPED_TEST_SUITE (ReadOfFmtIsIdentityT, Types);
  }

  TYPED_TEST (ReadOfFmtIsIdentityT, random_optional)
  {
    testing::read_of_fmt_is_identity
      (std::optional<TypeParam> {testing::random::value<TypeParam>{}()});
  }
}
