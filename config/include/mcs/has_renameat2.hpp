// Copyright (C) 2025-2026 Fraunhofer ITWM
// License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

#pragma once

// renameat2 is a Linux syscall, but only glibc provides a C wrapper.
// musl (used by Alpine Linux) does not provide renameat2.
//
#if defined (__GLIBC__)
#define MCS_CONFIG_HAS_RENAMEAT2
#endif
