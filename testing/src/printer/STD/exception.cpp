// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <mcs/testing/printer/STD/exception.hpp>
#include <mcs/testing/printer/define/via_fmt.hpp>
#include <mcs/util/FMT/STD/exception.hpp>

namespace std
{
  MCS_TESTING_PRINTER_DEFINE_VIA_FMT (exception_ptr);
}
