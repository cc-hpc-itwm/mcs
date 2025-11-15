// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>
#include <mcs/util/require_semi.hpp>

#define MCS_TESTING_PRINTER_DEFINE_VIA_FMT_IMPL(_type...)   \
  auto PrintTo                                              \
    ( _type const& x                                        \
    , std::ostream* os                                      \
    ) noexcept -> void                                      \
  {                                                         \
    *os << fmt::format ("{}", x);                           \
  } MCS_UTIL_REQUIRE_SEMI()
