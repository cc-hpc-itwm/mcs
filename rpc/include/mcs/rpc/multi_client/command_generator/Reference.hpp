// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <functional>
#include <type_traits>

namespace mcs::rpc::multi_client::command_generator
{
  // Stores a uses a reference to a command for each client.
  //
  template<typename T>
    struct Reference
  {
    constexpr explicit Reference
      ( std::reference_wrapper<T const>
      ) noexcept
      ;

    template<typename Client>
      constexpr auto operator()
        ( Client const&
        ) const noexcept -> std::reference_wrapper<T const>
      ;

  private:
    std::reference_wrapper<T const> _ref;
  };
}

#include "detail/Reference.ipp"
