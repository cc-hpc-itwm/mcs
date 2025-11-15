// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <iterator>

namespace mcs::rpc::multi_client::command_generator
{
  // Produces the elements from the given (forward iterator) range
  // [*begin, *++begin, ...)
  //
  template<typename R>
    struct Range
  {
    constexpr Range (R) noexcept;

    // Calling operator() more often then std::distance (std::cbegin
    // (range), std::cend (range)) yields undefined behavior
    //
    template<typename Client>
      constexpr auto operator()
        ( Client const&
        ) noexcept
      ;

  private:
    R _range;
    decltype (std::cbegin (_range)) _value {std::cbegin (_range)};
  };

  template<typename R>
    constexpr auto make_range (R) noexcept;
}

#include "detail/Range.ipp"
