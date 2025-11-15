// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mcs::util
{
  template<typename To, typename From>
    constexpr auto cast (From const& from) -> To
  {
    return Cast<To, From>{} (from);
  }
}

#define MCS_UTIL_DEFINE_POINTER_CAST(_const_to, _const_from)            \
  template<typename To, typename From>                                  \
    struct Cast<To _const_to*, From _const_from*>                       \
  {                                                                     \
    constexpr auto operator()                                           \
      ( From _const_from* from                                          \
      ) const -> To _const_to*                                          \
    {                                                                   \
      if constexpr (std::is_same_v<To, From>)                           \
      {                                                                 \
        return from;                                                    \
      }                                                                 \
                                                                        \
      if constexpr (std::is_void_v<From>)                               \
      {                                                                 \
        return static_cast<To _const_to*> (from);                       \
      }                                                                 \
                                                                        \
      if constexpr (std::is_same_v<From, std::byte>)                    \
      {                                                                 \
        return reinterpret_cast<To _const_to*> (from);                  \
      }                                                                 \
                                                                        \
      if constexpr (std::is_same_v<To, std::byte>)                      \
      {                                                                 \
        return reinterpret_cast<To _const_to*> (from);                  \
      }                                                                 \
                                                                        \
      /* \todo make explicit list of cases */                           \
      return reinterpret_cast<To _const_to*> (from);                    \
    }                                                                   \
  }

namespace mcs::util
{
  MCS_UTIL_DEFINE_POINTER_CAST (/* mutable */, /* mutable */);
  MCS_UTIL_DEFINE_POINTER_CAST (const        , /* mutable */);
  MCS_UTIL_DEFINE_POINTER_CAST (const        , const        );
}

#undef MCS_UTIL_DEFINE_POINTER_CAST

#define MCS_UTIL_DEFINE_POINTER_INTEGRAL_CAST(_maybe_const)       \
  template<std::integral I, typename P>                           \
    requires (detail::can_hold<I, P _maybe_const>)                \
    struct Cast<I, P _maybe_const*>                               \
    {                                                             \
      constexpr auto operator() (P _maybe_const* p) const -> I    \
      {                                                           \
        return reinterpret_cast<I> (p);                           \
      }                                                           \
    };                                                            \
                                                                  \
  template<typename P, std::integral I>                           \
    requires (detail::can_hold<I, P _maybe_const>)                \
  struct Cast<P _maybe_const*, I>                                 \
    {                                                             \
      constexpr auto operator() (I i) const -> P _maybe_const*    \
      {                                                           \
        return reinterpret_cast<P _maybe_const*> (i);             \
      }                                                           \
    }

namespace mcs::util
{
  MCS_UTIL_DEFINE_POINTER_INTEGRAL_CAST (const);
  MCS_UTIL_DEFINE_POINTER_INTEGRAL_CAST (/* mutable */);
}

#undef MCS_UTIL_DEFINE_POINTER_INTEGRAL_CAST

namespace mcs::util
{
  template<std::unsigned_integral To, std::signed_integral From>
    struct Cast<To, From>
  {
    constexpr auto operator() (From const& from) const -> To
    {
      if (std::cmp_greater (from, std::numeric_limits<To>::max()))
      {
        throw std::invalid_argument
          {fmt::format ( "cast signed to unsigned: {} > {}"
                       , from
                       , std::numeric_limits<To>::max()
                       )
          };
      }

      if (std::cmp_less (from, 0))
      {
        throw std::invalid_argument
          {fmt::format ("cast signed to unsigned: {} < 0", from)};
      }

      return static_cast<To> (from);
    }
  };
}

namespace mcs::util
{
  template<std::signed_integral To, std::unsigned_integral From>
    struct Cast<To, From>
  {
    constexpr auto operator() (From const& from) const -> To
    {
      if (std::cmp_greater (from, std::numeric_limits<To>::max()))
      {
        throw std::invalid_argument
          {fmt::format ( "cast unsigned to signed: {} > {}"
                       , from
                       , std::numeric_limits<To>::max()
                       )
          };
      }

      return static_cast<To> (from);
    }
  };
}

namespace mcs::util
{
  template<std::signed_integral To, std::signed_integral From>
    struct Cast<To, From>
  {
    constexpr auto operator() (From const& from) const -> To
    {
      if constexpr (std::is_same_v<To, From>)
      {
        return from;
      }

      if constexpr (sizeof (To) >= sizeof (From))
      {
        return static_cast<To> (from);
      }

      if (std::cmp_greater (from, std::numeric_limits<To>::max()))
      {
        throw std::invalid_argument
          {fmt::format ( "cast signed to signed: {} > {}"
                       , from
                       , std::numeric_limits<To>::max()
                       )
          };
      }

      return static_cast<To> (from);
    }
  };
}
