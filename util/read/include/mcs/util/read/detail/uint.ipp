// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/isdigit.hpp>
#include <mcs/util/read/define.hpp>
#include <mcs/util/read/skip_whitespace.hpp>
#include <stdexcept>

namespace mcs::util::read
{
  namespace detail
  {
    template<typename T, typename Char>
      constexpr auto uint (State<Char>& state) -> T
    {
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

      auto const as_uint
        { [&]
          {
            switch (state.head())
            {
              break; case '0': return T {0};
              break; case '1': return T {1};
              break; case '2': return T {2};
              break; case '3': return T {3};
              break; case '4': return T {4};
              break; case '5': return T {5};
              break; case '6': return T {6};
              break; case '7': return T {7};
              break; case '8': return T {8};
              break; case '9': return T {9};
            }

            throw std::logic_error {"isdigit and not in ['0'..'9']"};
          }
        };

      auto v {as_uint()};

      while (digit())
      {
        v *= T {10};
        v += as_uint();
      }

      return v;
    }
  }

  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::uint_least8_t)
  {
    return detail::uint<std::uint_least8_t> (state);
  }
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::uint_least16_t)
  {
    return detail::uint<std::uint_least16_t> (state);
  }
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::uint_least32_t)
  {
    return detail::uint<std::uint_least32_t> (state);
  }
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::uint_least64_t)
  {
    return detail::uint<std::uint_least64_t> (state);
  }
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::byte)
  {
    // \todo memcpy!?
    return std::byte {detail::uint<std::uint_least8_t> (state)};
  }
}
