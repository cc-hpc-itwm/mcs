// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <iosfwd>

#define MCS_TESTING_PRINTER_DECLARE_IMPL(_type...)                  \
  auto PrintTo (_type const&, std::ostream*) noexcept -> void
