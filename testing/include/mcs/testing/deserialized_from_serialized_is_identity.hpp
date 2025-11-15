// Copyright (C) 2022,2024-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <concepts>
#include <mcs/serialization/Concepts.hpp>

namespace mcs::testing
{
  template<typename T>
    concept serialization_identity_testable =
       serialization::is_serializable<T>
    && std::equality_comparable<T>
    ;

  template<serialization_identity_testable T>
    constexpr auto deserialized_from_serialized_is_identity (T const&);
}

#include "detail/deserialized_from_serialized_is_identity.ipp"
