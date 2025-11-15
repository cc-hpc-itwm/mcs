// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Declare a customization of mcs::testing::random::value for type wrappers.
//
// Requires
// - value<T::underlying_type> is defined
//
// EXAMPLE
//
//   namespace mcs::testing::random
//   {
//     template<>
//       MCS_TESTING_RANDOM_DECLARE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE
//         ( ns::type
//         );
//   }
//
#define MCS_TESTING_RANDOM_DECLARE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE(_type) \
  MCS_TESTING_RANDOM_DECLARE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE_IMPL (_type)

#include "detail/declare.ipp"
