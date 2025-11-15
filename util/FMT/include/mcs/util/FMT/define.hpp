// Copyright (C) 2022-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// Define customization of fmt
//
// EXAMPLE:
//
// namespace fmt
// {
//   MCS_UTIL_FMT_DEFINE_PARSE (ctx, ns::type)
//   {
//     return ctx.begin();
//   }
//   MCS_UTIL_FMT_DEFINE_FORMAT (value, ctx, ns::type)
//   {
//     return fmt::format_to (ctx.out(), "{}", value);
//   }
// }
//
#define MCS_UTIL_FMT_DEFINE_PARSE(_ctx, _type...)                         \
  MCS_UTIL_FMT_DEFINE_PARSE_IMPL(_ctx, _type)
#define MCS_UTIL_FMT_DEFINE_FORMAT(_variable, _ctx, _type...)             \
  MCS_UTIL_FMT_DEFINE_FORMAT_IMPL(_variable, _ctx, _type)

#include "detail/define.ipp"
