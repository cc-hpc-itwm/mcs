// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#include <mcs/testing/printer/declare.hpp>
#include <tuple>

namespace std
{
  template<typename... Ts> MCS_TESTING_PRINTER_DECLARE (tuple<Ts...>);
}

#include "detail/tuple.ipp"
