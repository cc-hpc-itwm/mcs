# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_syscall_Error)
target_include_directories (mcs_util_syscall_Error PUBLIC include)
target_link_libraries (mcs_util_syscall_Error
  PUBLIC mcs_error
  PRIVATE mcs_config
)

add_library (mcs_util_syscall)

target_include_directories (mcs_util_syscall PUBLIC include)
target_link_libraries (mcs_util_syscall
  PUBLIC mcs_util_syscall_Error
  PRIVATE mcs_config
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()

add_subdirectory (src)
