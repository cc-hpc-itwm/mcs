// Copyright (C) 2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

namespace mcs::util
{
  namespace detail
  {
    template<typename, typename... >
      struct same_invoke_result_type : std::false_type
    {};

    template<typename Visitor, typename Head, typename... Tail>
      struct same_invoke_result_type<Visitor, Head, Tail...>
        : std::bool_constant
           < ( std::same_as
                 < std::invoke_result_t<Visitor, Head>
                 , std::invoke_result_t<Visitor, Tail>
                 > && ...
             )
           >
    {};
  }

  template<typename Visitor, typename... Args>
    concept is_visitor = (std::invocable<Visitor, Args> && ...)
      && detail::same_invoke_result_type<Visitor, Args...>::value
      ;
}
