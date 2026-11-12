// Copyright (C) 2022-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <functional>
#include <limits>
#include <mcs/util/cast.hpp>
#include <mcs/util/isdigit.hpp>
#include <mcs/util/read/Read.hpp>
#include <mcs/util/read/State.hpp>
#include <mcs/util/read/skip_whitespace.hpp>
#include <stdexcept>
#include <type_traits>

namespace mcs::util::read
{
  namespace detail
  {
    template<typename UnsignedInteger, typename Char>
      constexpr auto unsigned_integer (State<Char>& state) -> UnsignedInteger
    {
      static_assert (std::is_unsigned_v<UnsignedInteger>);

      skip_whitespace (state);

      auto const digit
        { [&]
          {
            return !state.end() && isdigit (state.peek());
          }
        };

      if (!digit())
      {
        throw state.error ("Expected digit.");
      }

      auto const as_unsigned_integer
        { [&]
          {
            switch (state.head())
            {
              break; case '0': return UnsignedInteger {0};
              break; case '1': return UnsignedInteger {1};
              break; case '2': return UnsignedInteger {2};
              break; case '3': return UnsignedInteger {3};
              break; case '4': return UnsignedInteger {4};
              break; case '5': return UnsignedInteger {5};
              break; case '6': return UnsignedInteger {6};
              break; case '7': return UnsignedInteger {7};
              break; case '8': return UnsignedInteger {8};
              break; case '9': return UnsignedInteger {9};
            }

            throw std::logic_error {"isdigit and not in ['0'..'9']"};
          }
        };

      constexpr auto const maximum
        { std::numeric_limits<UnsignedInteger>::max()
        };

      auto value {as_unsigned_integer()};

      while (digit())
      {
        auto const next {as_unsigned_integer()};

        if (value > (maximum - next) / UnsignedInteger {10})
        {
          throw state.error ("UnsignedInteger out of range.");
        }

        value *= UnsignedInteger {10};
        value += next;
      }

      return value;
    }

    template<typename SignedInteger, typename Char>
      constexpr auto signed_integer (State<Char>& state) -> SignedInteger
    {
      static_assert (std::is_signed_v<SignedInteger>);

      skip_whitespace (state);

      auto const sign
        { std::invoke
          ( [&]() noexcept
            {
              if (!state.end() && state.peek() == '-')
              {
                std::ignore = state.head();

                return true;
              }

              return false;
            }
          )
        };

      using UnsignedInteger = std::make_unsigned_t<SignedInteger>;

      constexpr auto const maximum {std::numeric_limits<SignedInteger>::max()};
      constexpr auto const magnitude_of_minium
        { util::cast<UnsignedInteger> (maximum) + UnsignedInteger {1}
        };

      auto const value_magnitude {unsigned_integer<UnsignedInteger> (state)};

      if (sign && value_magnitude == magnitude_of_minium)
      {
        return std::numeric_limits<SignedInteger>::min();
      }

      auto const value {util::cast<SignedInteger> (value_magnitude)};

      return sign
        ? std::invoke (std::negate<SignedInteger>{}, value)
        : value
        ;
    }
  }

  template<typename Char>
    auto Read<std::uint_least8_t>::read
      ( State<Char>& state
      ) -> std::uint_least8_t
  {
    return detail::unsigned_integer<std::uint_least8_t> (state);
  }
  template<typename Char>
    auto Read<std::int_least8_t>::read
      ( State<Char>& state
      ) -> std::int_least8_t
  {
    return detail::signed_integer<std::int_least8_t> (state);
  }

  template<typename Char>
    auto Read<std::uint_least16_t>::read
      ( State<Char>& state
      ) -> std::uint_least16_t
  {
    return detail::unsigned_integer<std::uint_least16_t> (state);
  }
  template<typename Char>
    auto Read<std::int_least16_t>::read
      ( State<Char>& state
      ) -> std::int_least16_t
  {
    return detail::signed_integer<std::int_least16_t> (state);
  }

  template<typename Char>
    auto Read<std::uint_least32_t>::read
      ( State<Char>& state
      ) -> std::uint_least32_t
  {
    return detail::unsigned_integer<std::uint_least32_t> (state);
  }
  template<typename Char>
    auto Read<std::int_least32_t>::read
      ( State<Char>& state
      ) -> std::int_least32_t
  {
    return detail::signed_integer<std::int_least32_t> (state);
  }

  template<typename Char>
    auto Read<std::uint_least64_t>::read
      ( State<Char>& state
      ) -> std::uint_least64_t
  {
    return detail::unsigned_integer<std::uint_least64_t> (state);
  }
  template<typename Char>
    auto Read<std::int_least64_t>::read
      ( State<Char>& state
      ) -> std::int_least64_t
  {
    return detail::signed_integer<std::int_least64_t> (state);
  }

  template<typename Char>
    auto Read<std::byte>::read
      ( State<Char>& state
      ) -> std::byte
  {
    // \todo memcpy!?
    return std::byte {detail::unsigned_integer<std::uint_least8_t> (state)};
  }
}
