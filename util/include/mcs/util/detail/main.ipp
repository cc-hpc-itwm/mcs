// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <mcs/util/FMT/STD/exception.hpp>
#include <mcs/util/FMT/print_noexcept.hpp>
#include <mcs/util/cast.hpp>
#include <utility>

namespace mcs::util
{
  template<is_main_implementation Main>
    auto main (int argc, char const** argv, Main&& main) noexcept -> int
  try
  {
    return std::invoke ( std::forward<Main> (main)
                       , Args {argv, cast<Args::size_type> (argc)}
                       );
  }
  catch (...)
  {
    FMT::print_noexcept (stderr, "Error: {}\n", std::current_exception());

    return EXIT_FAILURE;
  }
}
