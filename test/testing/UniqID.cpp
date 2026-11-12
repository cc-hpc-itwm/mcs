// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/testing/UniqID.hpp>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/CIdent.hpp>
#include <optional>
#include <string>

namespace mcs::testing
{
  struct UniqIDF : public mcs::testing::random::Test{};

  TEST_F (UniqIDF, id_has_given_prefix)
  {
    auto prefix {random::value<random::CIdent>{}()};
    UniqID id {prefix};

    ASSERT_TRUE (id().starts_with (prefix));
  }

  TEST_F (UniqIDF, digest_can_be_extended)
  {
    auto prefix {random::value<random::CIdent>{}()};
    auto id {UniqID {prefix}};
    auto digest {id.digest()};
    auto const original {fmt::format ("{}", digest)};

    ASSERT_EQ (original.size(), 32UL);
    digest << "additional information";
    ASSERT_NE (fmt::format ("{}", digest), original);
    ASSERT_EQ (fmt::format ("{}", id.digest()), original);
  }

  TEST_F (UniqIDF, digest_accepts_optional_values)
  {
    auto missing {UniqID::Digest {}};
    auto empty {UniqID::Digest {}};
    missing << std::optional<std::string> {};
    empty << std::optional<std::string> {std::string {}};

    ASSERT_NE (fmt::format ("{}", missing), fmt::format ("{}", empty));
  }
}
