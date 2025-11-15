// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <cassert>
#include <cstddef>
#include <mcs/util/type/detail/run.hpp>
#include <utility>

namespace mcs::util::type::detail
{
  template<typename...> struct Run;

  template<typename Head>
    struct Run<Head>
  {
    template<typename Runner, typename... Args>
      constexpr auto operator()
        ( std::size_t i
        , Runner&& runner
        , Args&&... args
        ) const
    {
      assert (i == 0); std::ignore = i;

      return detail::run<Head>
        ( std::forward<Runner> (runner)
        , std::forward<Args> (args)...
        );
    }
  };

  template<typename Head, typename... Tail>
    struct Run<Head, Tail...>
  {
    template<typename Runner, typename... Args>
      constexpr auto operator()
        ( std::size_t i
        , Runner&& runner
        , Args&&... args
        ) const
    {
      if (i == 0)
      {
        return detail::run<Head>
          ( std::forward<Runner> (runner)
          , std::forward<Args> (args)...
          );
      }
      else
      {
        return Run<Tail...>{}
          ( i - 1
          , std::forward<Runner> (runner)
          , std::forward<Args> (args)...
          );
      }
    }
  };
}
