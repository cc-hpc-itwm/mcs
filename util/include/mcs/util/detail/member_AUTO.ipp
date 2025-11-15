// Copyright (C) 2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_UTIL_MEMBER_AUTO_IMPL(_name, _expr...)      \
  decltype (_expr) _name {_expr}
