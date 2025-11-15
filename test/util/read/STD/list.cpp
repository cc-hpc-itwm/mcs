// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <algorithm>
#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/UTIL/string.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/read/STD/list.hpp>
#include <mcs/util/read/uint.hpp>
#include <mcs/util/string.hpp>

namespace mcs::util::read
{
  TEST (ReadOfFmtIsIdentity, empty_list)
  {
    testing::read_of_fmt_is_identity (std::list<unsigned int>{});
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

  TYPED_TEST (ReadOfFmtIsIdentityT, random_list)
  {
    auto value {testing::random::value<TypeParam>{}};
    auto xs {std::list<TypeParam>{}};
    std::generate_n
      ( std::back_inserter (xs)
      , testing::random::value<decltype (xs.size())> {0, 100}()
      , [&]
        {
          return value();
        }
      );

    testing::read_of_fmt_is_identity (xs);
  }
}
