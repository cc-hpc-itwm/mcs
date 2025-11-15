// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Define a test printer using an existing instance of fmt::formatter.
//
// EXAMPLE
//
//    #include <ns/S.hpp>
//
//    namespace ns
//    {
//      MCS_TESTING_PRINTER_DEFINE_VIA_FMT (S);
//    }
//
#define MCS_TESTING_PRINTER_DEFINE_VIA_FMT(_type...)        \
  MCS_TESTING_PRINTER_DEFINE_VIA_FMT_IMPL(_type)

#include "detail/via_fmt.ipp"
