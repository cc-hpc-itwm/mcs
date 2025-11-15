// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Define a non-intrusive customization of Read.
//
// EXAMPLE:
//
//     namespace mcs::util::read
//     {
//       MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION (state, ns::type)
//       {
//         return ... (state);
//       }
//     }
//
#define MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION(_state, _type...) \
  MCS_UTIL_READ_DEFINE_NONINTRUSIVE_IMPLEMENTATION_IMPL(_state, _type)

#include "detail/define.ipp"
