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

namespace mcs::util
{
  template<typename To, typename From>
    struct Cast<To*, From*>
  {
    constexpr auto operator()
      ( From* from
      ) const -> To*
    {
      if constexpr (std::is_same_v<To, From>)
      {
        return from;
      }

      if constexpr (std::is_void_v<From>)
      {
        return static_cast<To*> (from);
      }

      if constexpr (std::is_same_v<From, std::byte>)
      {
        return reinterpret_cast<To*> (from);
      }

      if constexpr (std::is_same_v<To, std::byte>)
      {
        return reinterpret_cast<To*> (from);
      }

      /* \todo make explicit list of cases */
      return reinterpret_cast<To*> (from);
    }
  };

  template<typename To, typename From>
    struct Cast<To const*, From*>
  {
    constexpr auto operator()
      ( From* from
      ) const -> To const*
    {
      if constexpr (std::is_same_v<To, From>)
      {
        return from;
      }

      if constexpr (std::is_void_v<From>)
      {
        return static_cast<To const*> (from);
      }

      if constexpr (std::is_same_v<From, std::byte>)
      {
        return reinterpret_cast<To const*> (from);
      }

      if constexpr (std::is_same_v<To, std::byte>)
      {
        return reinterpret_cast<To const*> (from);
      }

      /* \todo make explicit list of cases */
      return reinterpret_cast<To const*> (from);
    }
  };

  template<typename To, typename From>
    struct Cast<To const*, From const*>
  {
    constexpr auto operator()
      ( From const* from
      ) const -> To const*
    {
      if constexpr (std::is_same_v<To, From>)
      {
        return from;
      }

      if constexpr (std::is_void_v<From>)
      {
        return static_cast<To const*> (from);
      }

      if constexpr (std::is_same_v<From, std::byte>)
      {
        return reinterpret_cast<To const*> (from);
      }

      if constexpr (std::is_same_v<To, std::byte>)
      {
        return reinterpret_cast<To const*> (from);
      }

      /* \todo make explicit list of cases */
      return reinterpret_cast<To const*> (from);
    }
  };
}

namespace mcs::util
{
  template<std::integral I, typename P>
    requires (detail::can_hold<I, P const>)
    struct Cast<I, P const*>
    {
      constexpr auto operator() (P const* p) const -> I
      {
        return reinterpret_cast<I> (p);
      }
    };

  template<typename P, std::integral I>
    requires (detail::can_hold<I, P const>)
    struct Cast<P const*, I>
    {
      constexpr auto operator() (I i) const -> P const*
      {
        return reinterpret_cast<P const*> (i);
      }
    };

  template<std::integral I, typename P>
    requires (detail::can_hold<I, P>)
    struct Cast<I, P*>
    {
      constexpr auto operator() (P* p) const -> I
      {
        return reinterpret_cast<I> (p);
      }
    };

  template<typename P, std::integral I>
    requires (detail::can_hold<I, P>)
    struct Cast<P*, I>
    {
      constexpr auto operator() (I i) const -> P*
      {
        return reinterpret_cast<P*> (i);
      }
    };
}

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
