// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <list>
#include <mcs/testing/printer/declare.hpp>

namespace std
{
  template<typename T> MCS_TESTING_PRINTER_DECLARE (list<T>);
}

#include "detail/list.ipp"
