// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <type_traits>

namespace mcs::util::heterogeneous_map
{
  template<typename Key>
    concept is_key = std::is_default_constructible_v<Key>
      && requires (Key& key)
         {
           { ++key
           } -> std::convertible_to<Key&>
           ;
         }
    ;
}
