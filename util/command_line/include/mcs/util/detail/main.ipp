// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/arguments/View.hpp>
#include <mcs/util/cast.hpp>
#include <utility>

namespace mcs::util
{
  template<is_help_implementation Help, is_main_implementation Main>
    auto main
      ( int argc
      , char const** argv
      , Help&& help
      , Main&& main_implementation
      ) noexcept -> int
  try
  {
    auto arguments {arguments::View {argv, cast<std::size_t> (argc)}};

    if (arguments.help_requested())
    {
      std::invoke (std::forward<Help> (help), std::move (arguments));

      return EXIT_SUCCESS;
    }

    return std::invoke ( std::forward<Main> (main_implementation)
                       , std::move (arguments)
                       );
  }
  catch (...)
  {
    FMT::print_noexcept (stderr, "Error: {}\n", std::current_exception());

    return EXIT_FAILURE;
  }
}
