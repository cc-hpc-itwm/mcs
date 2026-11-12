// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <compare>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <mcs/testing/random/Test.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <mcs/testing/read_of_fmt_is_identity.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/integer.hpp>
#include <mcs/util/read/parse.hpp>
#include <mcs/util/read/prefix.hpp>

namespace
{
  struct UDT
  {
    constexpr explicit UDT (unsigned int value) noexcept
      : _value {value}
    {}

    struct has_intrusive_ctor_from_read_State;
    template<typename Char>
      explicit UDT (mcs::util::read::State<Char>&);

    constexpr auto operator<=> (UDT const&) const noexcept = default;

  private:
    unsigned int _value;

    friend struct fmt::formatter<UDT>;
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
  template<>
    struct formatter<UDT>
  {
    template<typename ParseContext>
      constexpr auto parse (ParseContext&);

    template<typename FormatContext>
      constexpr auto format
        ( UDT const&
        , FormatContext& context
        ) const -> decltype (context.out());
  };

  template<typename ParseContext>
    constexpr auto formatter<UDT>::parse (ParseContext& context)
  {
    return context.begin();
  }
  template<typename FormatContext>
    constexpr auto formatter<UDT>::format
      ( UDT const& x
      , FormatContext& context
      ) const -> decltype (context.out())
  {
    return fmt::format_to (context.out(), "udt {}", x._value);
  }
}

TEST_F (ReadOfFmtIsIdentityR, intrusive)
{
  mcs::testing::read_of_fmt_is_identity
    (UDT {mcs::testing::random::value<unsigned int>{}()});
}
