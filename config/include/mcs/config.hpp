// Copyright (C) 2023-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#if defined (__GNUC__) && (__GNUC__ == 11)
#define MCS_CONFIG_GCC_WORKAROUND_BROKEN_TEMPLATE_FORWARD_DECLARATION
// see https://godbolt.org/z/fhrKor51E
#endif

#if defined (__GNUC__) && (__GNUC__ == 11)
#define MCS_CONFIG_GCC_WORKAROUND_BROKEN_RANGES_SORT
#endif

#if defined (__GNUC__) && (  __GNUC__ == 12                           \
                          || __GNUC__ == 13                           \
                          || __GNUC__ == 14                           \
                          )
#define MCS_CONFIG_GCC_WORKAROUND_FALSE_WARNING_WITH_DEFAULT_COMPARE
#endif

#if defined (__GNUC__) && (__GNUC__ == 13)
#define MCS_CONFIG_GCC_WORKAROUND_FALSE_WARNING_WITH_RANGES_COMPARE
#endif

#if defined (__clang__) && (  __clang_major__ == 20                   \
                           || __clang_major__ == 21                   \
                           )
#define MCS_CONFIG_CLANG_TEMPLATED_DTOR_PARSER_IS_BROKEN
#endif
