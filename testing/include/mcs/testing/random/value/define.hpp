// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Define a customization of mcs::testing::random::value for type wrappers.
//
// EXAMPLE
//
//   namespace mcs::testing::random
//   {
//     MCS_TESTING_RANDOM_DEFINE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE
//       ( ns::type
//       , ns::make_type
//       );
//   }
//
#define MCS_TESTING_RANDOM_DEFINE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE(_type, _make...) \
  MCS_TESTING_RANDOM_DEFINE_VALUE_VIA_MAKE_FROM_UNDERLYING_TYPE_IMPL (_type, _make)

#include "detail/define.ipp"
