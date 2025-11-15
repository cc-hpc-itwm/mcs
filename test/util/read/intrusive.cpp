// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <compare>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/FMT/access.hpp>
#include <mcs/util/FMT/declare.hpp>
#include <mcs/util/FMT/define.hpp>
#include <mcs/util/read/declare.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/parse.hpp>
#include <mcs/util/read/prefix.hpp>
#include <mcs/util/read/uint.hpp>

namespace
{
  struct UDT
  {
    constexpr explicit UDT (unsigned int value) noexcept
      : _value {value}
    {}

    MCS_UTIL_READ_DECLARE_INTRUSIVE_CTOR (UDT);

    constexpr auto operator<=> (UDT const&) const noexcept = default;

  private:
    unsigned int _value;

    MCS_UTIL_FMT_ACCESS();
  };

  template<typename Char>
    UDT::UDT (mcs::util::read::State<Char>& state)
      : _value { mcs::util::read::parse<decltype (_value)>
                   (mcs::util::read::prefix (state, "udt"))
               }
  {}

  struct ReadOfFmtIsIdentityR : public mcs::testing::random::Test{};
}

namespace fmt
{
  template<> MCS_UTIL_FMT_DECLARE (UDT);

  MCS_UTIL_FMT_DEFINE_PARSE (ctx, UDT)
  {
    return ctx.begin();
  }
  MCS_UTIL_FMT_DEFINE_FORMAT (x, ctx, UDT)
  {
    return fmt::format_to (ctx.out(), "udt {}", x._value);
  }
}

TEST_F (ReadOfFmtIsIdentityR, intrusive)
{
  mcs::testing::read_of_fmt_is_identity
    (UDT {mcs::testing::random::value<unsigned int>{}()});
}
