// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/define.hpp>
#include <mcs/util/read/maybe.hpp>

namespace mcs::util::read
{
  namespace detail
  {
    template<typename Variant, is_readable... Ts> struct VariantElement;

    template<typename Variant> struct VariantElement<Variant>
    {
      template<typename Char>
        constexpr auto operator() (State<Char>& state) const -> Variant
      {
        throw state.error ("read variant: none of the alternatives");
      }
    };

    template<typename Variant, is_readable T, is_readable... Ts>
      struct VariantElement<Variant, T, Ts...> : public VariantElement<Variant, Ts...>
    {
      template<typename Char>
        constexpr auto operator() (State<Char>& state) const -> Variant
      {
        if (auto value {maybe<T> (state)})
        {
          return *value;
        }
        else
        {
          return VariantElement<Variant, Ts...>::operator() (state);
        }
      }
    };
  }

  template<is_readable... Ts>
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION
      ( state
      , std::variant<Ts...>
      )
  {
    return detail::VariantElement<std::variant<Ts...>, Ts...>{} (state);
  }
}
