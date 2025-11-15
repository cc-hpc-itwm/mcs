// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/util/read/Concepts.hpp>

// Declare an non-intrusive customization of Read.
//
// EXAMPLE
//
//     namespace mcs::util::read
//     {
//       template<> MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION
//         ( ns::type
//         );
//     }
//
//
#define MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION(_type...)      \
  MCS_UTIL_READ_DECLARE_NONINTRUSIVE_IMPLEMENTATION_IMPL(_type)

// Declare an intrusive parse constructor.
//
// EXAMPLE
//
//     struct S
//     {
//       MCS_UTIL_READ_DECLARE_INTRUSIVE_CTOR (S);
//     }
//
//
#define MCS_UTIL_READ_DECLARE_INTRUSIVE_CTOR(_type)                      \
  MCS_UTIL_READ_DECLARE_INTRUSIVE_CTOR_IMPL(_type)

#include "detail/declare.ipp"
