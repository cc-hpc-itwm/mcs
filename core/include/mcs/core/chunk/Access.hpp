// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/core/chunk/access/Const.hpp>
#include <mcs/core/chunk/access/Mutable.hpp>
#include <type_traits>

namespace mcs::core::chunk
{
  template<typename A>
    concept is_access = std::is_same_v<A, access::Const>
                     || std::is_same_v<A, access::Mutable>
    ;

  // Select a value for the given access
  //
  // EXAMPLE
  //
  //      auto const flags
  //        { select<Access>
  //          ( make_value<access::Const> (PROT_READ)
  //          , make_value<access::Mutable> (PROT_READ | PROT_WRITE)
  //          )
  //        };
  //
  template<is_access A, typename T>
    constexpr auto select ( typename access::Const::Value<T>
                          , typename access::Mutable::Value<T>
                          ) -> T;

  template<is_access A, typename T>
    constexpr auto make_value (T value) -> typename A::template Value<T>;
}

#include "detail/Access.ipp"
