// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <mcs/rpc/multi_client/command_generator/Reference.hpp>
#include <type_traits>

namespace mcs::rpc::multi_client::command_generator
{
  // Stores a uses a command for each client.
  //
  template<typename T>
    struct Const
  {
    template<typename... Args>
      requires (std::is_constructible_v<T, Args...>)
      constexpr explicit Const
        ( Args&&...
        ) noexcept (std::is_nothrow_constructible_v<T, Args...>)
      ;

    template<typename Client>
      constexpr auto operator()
        ( Client const&
        ) const noexcept -> std::reference_wrapper<T const>
      ;

  private:
    T _value;
    Reference<T> _ref {_value};
  };
}

#include "detail/Const.ipp"
