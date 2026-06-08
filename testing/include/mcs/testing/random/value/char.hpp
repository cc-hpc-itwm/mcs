// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cstdint>
#include <mcs/testing/random/value.hpp>
#include <mcs/testing/random/value/integral.hpp>
#include <type_traits>

namespace mcs::testing::random
{
  namespace detail
  {
    template<typename T>
      using CharacterValueBase = value
        < std::conditional_t
          < std::is_signed_v<T>
          , std::make_signed_t<T>
          , std::make_unsigned_t<T>
          >
        >;
  }

  // produces random characters
  //
  template<>
    struct value<char> : public value<std::int_least8_t>
  {
    using Base = value<std::int_least8_t>;

    using Base::Min;
    using Base::Max;
    using Base::Base;

    auto operator()() -> char
    {
      return Base::operator()();
    }
  };

  template<>
    struct value<wchar_t> : public detail::CharacterValueBase<wchar_t>
  {
    using Base = detail::CharacterValueBase<wchar_t>;

    using Base::Min;
    using Base::Max;
    using Base::Base;

    auto operator()() -> wchar_t
    {
      return static_cast<wchar_t> (Base::operator()());
    }
  };

  template<>
    struct value<char16_t> : public detail::CharacterValueBase<char16_t>
  {
    using Base = detail::CharacterValueBase<char16_t>;

    using Base::Min;
    using Base::Max;
    using Base::Base;

    auto operator()() -> char16_t
    {
      return static_cast<char16_t> (Base::operator()());
    }
  };

  template<>
    struct value<char32_t> : public detail::CharacterValueBase<char32_t>
  {
    using Base = detail::CharacterValueBase<char32_t>;

    using Base::Min;
    using Base::Max;
    using Base::Base;

    auto operator()() -> char32_t
    {
      return static_cast<char32_t> (Base::operator()());
    }
  };
}
