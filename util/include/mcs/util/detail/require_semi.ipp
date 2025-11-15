// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

namespace mcs::util
{
  constexpr auto require_semi() noexcept -> void;
}

#define MCS_UTIL_REQUIRE_SEMI_IMPL()                 \
  using mcs::util::require_semi
