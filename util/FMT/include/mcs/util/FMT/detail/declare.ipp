// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#include <fmt/format.h>

#define MCS_UTIL_FMT_DECLARE_FORMATTER_IMPL(_type...)            \
  struct formatter<_type>

#define MCS_UTIL_FMT_DECLARE_FORMATTER_MEMBERS_IMPL(_type...)    \
    template<typename ParseContext>                              \
      constexpr auto parse (ParseContext&);                      \
                                                                 \
    template<typename FormatContext>                             \
      constexpr auto format                                      \
        ( _type const&                                           \
        , FormatContext& ctx                                     \
        ) const -> decltype (ctx.out())

#define MCS_UTIL_FMT_DECLARE_IMPL(_type...)                      \
  MCS_UTIL_FMT_DECLARE_FORMATTER (_type)                         \
  {                                                              \
    MCS_UTIL_FMT_DECLARE_FORMATTER_MEMBERS (_type);              \
  }
