// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/ranges.h>
#include <mcs/testing/printer/define/via_fmt.hpp>

namespace std
{
  template<typename... Ts> MCS_TESTING_PRINTER_DEFINE_VIA_FMT (tuple<Ts...>);
}
