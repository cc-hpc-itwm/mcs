// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

namespace mcs::util
{
  template<typename To, typename From> struct Cast;

  template<typename To, typename From>
    constexpr auto cast (From const&) -> To;
}

#include <concepts>
#include <mcs/config.hpp>

#if not defined (MCS_CONFIG_GCC_WORKAROUND_BROKEN_TEMPLATE_FORWARD_DECLARATION)
namespace mcs::util
{
  // Cast between signed and unsigned integrals. If the conversion is
  // invalid, those will either not compile or throw at execution
  // time.
  //
  template<std::unsigned_integral To, std::signed_integral From>
    struct Cast<To, From>;

  template<std::signed_integral To, std::unsigned_integral From>
    struct Cast<To, From>;

  template<std::signed_integral To, std::signed_integral From>
    struct Cast<To, From>;
}
#endif

namespace mcs::util
{
  // Cast pointers from one type to another.
  //
  template<typename To, typename From> struct Cast<To      *, From      *>;
  template<typename To, typename From> struct Cast<To const*, From      *>;
  template<typename To, typename From> struct Cast<To const*, From const*>;
}

namespace mcs::util
{
  // Cast between pointers and unsigned integrals.
  //
  // EXAMPLE:
  //   auto const x {42};
  //   auto const xiaddr {mcs::util::cast<std::uintptr_t> (std::addressof (x))};
  //   auto const y {*util::cast<int const*> (xiaddr)};
  //   ASSERT_EQ (x, y);
  //
  namespace detail
  {
    template<typename I, typename P>
      concept can_hold = ! (sizeof (I) < sizeof (P*));
  }

  template<std::integral I, typename P>
    requires (detail::can_hold<I, P>)       struct Cast<I       , P      *>;
  template<std::integral I, typename P>
    requires (detail::can_hold<I, P const>) struct Cast<I       , P const*>;
  template<typename P, std::integral I>
    requires (detail::can_hold<I, P>)       struct Cast<P      *, I       >;
  template<typename P, std::integral I>
    requires (detail::can_hold<I, P const>) struct Cast<P const*, I       >;
}

#include "detail/cast.ipp"
