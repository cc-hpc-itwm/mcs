// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <type_traits>

namespace mcs::rpc::multi_client::command_generator
{
  // Produces the sequence [initial_value, ++intial_value, ...]
  //
  template<typename T>
    struct Sequence
  {
    constexpr Sequence (T) noexcept (std::is_nothrow_move_constructible_v<T>);

    template<typename Client>
      constexpr auto operator()
        ( Client const&
        ) noexcept -> T
      ;

  private:
    T _value;
  };
}

#include "detail/Sequence.ipp"
