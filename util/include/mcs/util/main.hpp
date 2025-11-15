// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <span>

namespace mcs::util
{
  using Args = std::span<char const*>;

  template<typename Main>
    concept is_main_implementation = std::invocable<Main, Args>
    && std::convertible_to<std::invoke_result_t<Main, Args>, int>
    ;

  // Helper that creates Args from the command line and calls the
  // provided main implementation. If the main implementation throws
  // an exception, then the exception is caught and printed to stderr
  // and the return value of main will be EXIT_FAILURE.
  //
  // EXAMPLE:
  //
  // namespace
  // {
  //   auto main_impl (mcs::util::Args args) -> int
  //   {
  //     ...
  //     return EXIT_CODE;
  //   }
  // }
  //
  // auto main (int argc, char const** argv) noexcept -> int
  // {
  //   return mcs::util::main (argc, argv, main_impl);
  // }
  //
  // EXAMPLE:
  //
  // auto main (int argc, char const** argv) noexcept -> int
  // {
  //   return mcs::util::main
  //     ( argc, argv
  //     , [] (mcs::util::Args args)
  //       {
  //         ...
  //         return EXIT_CODE;
  //       }
  //     );
  // }
  //
  template<is_main_implementation Main>
    auto main (int, char const**, Main&&) noexcept -> int;
}

#include "detail/main.ipp"
