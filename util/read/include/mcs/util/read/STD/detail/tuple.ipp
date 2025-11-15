// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/InBrackets.hpp>
#include <mcs/util/read/Symbol.hpp>
#include <mcs/util/read/define.hpp>

namespace mcs::util::read
{
  namespace detail
  {
    template<typename Separator, is_readable...> struct TupleElements;

    template<typename Separator>
      struct TupleElements<Separator>
    {
      constexpr explicit TupleElements (Separator separator) noexcept
        : _separator {separator}
      {}

      template<typename Char>
        [[nodiscard]] constexpr auto operator() (State<Char>&) const noexcept
      {
        return std::make_tuple();
      }

    protected:
      Separator _separator;
    };

    template<typename Separator, is_readable T>
      struct TupleElements<Separator, T> : public TupleElements<Separator>
    {
      constexpr explicit TupleElements (Separator separator) noexcept
        : TupleElements<Separator> {separator}
      {}

      template<typename Char>
        [[nodiscard]] constexpr auto operator()
          ( State<Char>& state
          ) const
      {
        return std::make_tuple (parse<T> (state));
      }
    };

    template<typename Separator, is_readable T, is_readable... Ts>
      struct TupleElements<Separator, T, Ts...>
        : public TupleElements<Separator, Ts...>
    {
      constexpr explicit TupleElements (Separator separator) noexcept
        : TupleElements<Separator, Ts...> {separator}
      {}

      template<typename Char>
        [[nodiscard]] constexpr auto operator()
          ( State<Char>& state
          ) const
      {
        auto value {parse<T> (state)};
        TupleElements<Separator, Ts...>::_separator (state);

        return std::tuple_cat
          ( std::make_tuple (std::move (value))
          , TupleElements<Separator, Ts...>::operator() (state)
          );
      }
    };

    template<is_readable... Ts, typename Separator>
      constexpr auto tuple_elements (Separator separator) noexcept
    {
      return TupleElements<Separator, Ts...> {separator};
    }
  }

  template<is_readable... Ts>
    MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, std::tuple<Ts...>)
  {
    return in_brackets
      ( symbol ("(")
      , detail::tuple_elements<Ts...> (symbol (","))
      , symbol (")")
      ) (state)
      ;
  }
}
