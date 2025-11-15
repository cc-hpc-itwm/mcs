// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#define MCS_UTIL_FMT_DEFINE_PARSE_IMPL(_ctx, _type...)                     \
  template<typename ParseContext>                                          \
    constexpr auto formatter<_type>::parse (ParseContext& _ctx)

#define MCS_UTIL_FMT_DEFINE_FORMAT_IMPL(_variable, _ctx, _type...)         \
  template<typename FormatContext>                                         \
    constexpr auto formatter<_type>::format                                \
      ( _type const& _variable                                             \
      , FormatContext& _ctx                                                \
      ) const -> decltype (_ctx.out())
