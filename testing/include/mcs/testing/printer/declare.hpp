// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Declare a test printer.
//
// EXAMPLE
//
//    #include <ns/S.hpp>
//
//    namespace ns
//    {
//      MCS_TESTING_PRINTER_DECLARE (S);
//    }
//
#define MCS_TESTING_PRINTER_DECLARE(_type...)       \
  MCS_TESTING_PRINTER_DECLARE_IMPL(_type)

#include "detail/declare.ipp"
