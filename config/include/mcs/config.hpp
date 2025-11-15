// Copyright (C) 2023-2025 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

#if defined (__GNUC__) && (__GNUC__ == 11)
#define MCS_CONFIG_GCC_WORKAROUND_BROKEN_TEMPLATE_FORWARD_DECLARATION
// see https://godbolt.org/z/fhrKor51E
#endif

#if defined (__GNUC__) && (__GNUC__ == 12)
#define MCS_CONFIG_GCC_WORKAROUND_BROKEN_DEFAULT_CONSTRUCTOR_LINKAGE
#endif

#if defined (__clang__) && (  __clang_major__ == 19                   \
                           || __clang_major__ == 20                   \
                           || __clang_major__ == 21                   \
                           )
#define MCS_CONFIG_CLANG_TEMPLATED_DTOR_PARSER_IS_BROKEN
#endif
