// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/State.hpp>
#include <mcs/util/read/maybe.hpp>
#include <mcs/util/type/detail/run.hpp>
#include <utility>

namespace mcs::util::type::detail
{
  template<template<typename> typename, typename...> struct ParseIDAndRun;

  template<template<typename> typename Parse, typename Head>
    struct ParseIDAndRun<Parse, Head>
  {
    template<typename Char, typename Runner, typename... Args>
      auto operator()
        ( util::read::State<Char>& state
        , Runner&& runner
        , Args&&... args
        )
    {
      if (auto x {util::read::maybe<typename Parse<Head>::type> (state)})
      {
        return run<Head>
          ( std::forward<Runner> (runner)
          , std::move (*x)
          , std::forward<Args> (args)...
          );
      }

      // \todo collect help strings of the alternatives
      throw state.error ("Not any of the alternatives");
    }
  };

  template<template<typename> typename Parse, typename Head, typename... Tail>
    struct ParseIDAndRun<Parse, Head, Tail...>
  {
    template<typename Char, typename Runner, typename... Args>
      auto operator()
        ( util::read::State<Char>& state
        , Runner&& runner
        , Args&&... args
        )
    {
      if (auto x {util::read::maybe<typename Parse<Head>::type> (state)})
      {
        return run<Head>
          ( std::forward<Runner> (runner)
          , std::move (*x)
          , std::forward<Args> (args)...
          );
      }
      else
      {
        return ParseIDAndRun<Parse, Tail...>{}
          ( state
          , std::forward<Runner> (runner)
          , std::forward<Args> (args)...
          );
      }
    }
  };
}
