// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/util/arguments/View.hpp>
#include <type_traits>

namespace mcs::util
{
  template<typename Help>
    concept is_help_implementation = std::invocable<Help, arguments::View>
      && std::same_as<std::invoke_result_t<Help, arguments::View>, void>
      ;

  template<typename Main>
    concept is_main_implementation = std::invocable<Main, arguments::View>
    && std::convertible_to<std::invoke_result_t<Main, arguments::View>, int>
    ;

  // Helper that creates arguments::View from the command line,
  // invokes the help implementation when help was requested, and
  // otherwise calls the main implementation. If the main
  // implementation throws an exception, then the exception is caught
  // and printed to stderr and the return value of main will be
  // EXIT_FAILURE. The help callback is supposed to describe the
  // program's arguments. It runs before argument validation or
  // application side effects.
  //
  // EXAMPLE:
  //
  // namespace
  // {
  //   auto help_impl (mcs::util::arguments::View args) -> void
  //   {
  //     fmt::print ("Usage: {} input_file output_file\n", args.at (0));
  //   }
  //
  //   auto main_impl (mcs::util::arguments::View args) -> int
  //   {
  //     ...
  //     return EXIT_CODE;
  //   }
  // }
  //
  // auto main (int argc, char const** argv) noexcept -> int
  // {
  //   return mcs::util::main (argc, argv, help_impl, main_impl);
  // }
  //
  // EXAMPLE:
  //
  // auto main (int argc, char const** argv) noexcept -> int
  // {
  //   return mcs::util::main
  //     ( argc
  //     , argv
  //     , [] (mcs::util::arguments::View args)
  //       {
  //         fmt::print ("Usage: {} input_file output_file\n", args.at (0));
  //       }
  //     , [] (mcs::util::arguments::View args) -> int
  //       {
  //         ...
  //         return EXIT_CODE;
  //       }
  //     );
  // }
  //
  template<is_help_implementation Help, is_main_implementation Main>
    auto main (int, char const**, Help&&, Main&&) noexcept -> int;

}

#include "detail/main.ipp"
