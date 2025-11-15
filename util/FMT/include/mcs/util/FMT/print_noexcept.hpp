// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <fmt/base.h>
#include <mcs/util/FMT/Concepts.hpp>

namespace mcs::util::FMT
{
  // Prints to a stream and catches and ignores(!!) all exceptions
  // that happen during printing.
  //
  // EXAMPLE:
  //     struct S
  //     {
  //       ~S() noexcept
  //       {
  //         mcs::util::FMT::print_noexcept
  //           ( stderr
  //           , "~S(): {}"
  //           , std::current_exception()
  //           );
  //       }
  //     }
  //
  template<fmt::formattable... Args>
    constexpr auto print_noexcept
      ( util::FMT::sink auto
      , fmt::format_string<Args...>
      , Args&&...
      ) noexcept -> void
    ;
}

#include "detail/print_noexcept.ipp"
