// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/util/read/Read.hpp>
#include <type_traits>

namespace mcs::util::read
{
  namespace detail
  {
    template<typename T, typename = void>
      struct has_intrusive_ctor_from_read_State : public std::false_type{};
    template<typename T>
      struct has_intrusive_ctor_from_read_State
       < T
       , std::void_t<typename T::has_intrusive_ctor_from_read_State>
       > : public std::true_type{}
      ;
  }

  template<typename T, typename Char>
    constexpr auto parse (State<Char>& state) -> T
  {
    if constexpr (detail::has_intrusive_ctor_from_read_State<T>{})
    {
      return T {state};
    }
    else
    {
      return Read<T>::read (state);
    }
  }
}
