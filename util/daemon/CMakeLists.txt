# Copyright (C) 2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_daemon)
target_include_directories (mcs_util_daemon PUBLIC include)
target_link_libraries (mcs_util_daemon
  PUBLIC fmt
  PUBLIC mcs_error
  PUBLIC mcs_nonstd_scope
  PUBLIC mcs_serialization
  PUBLIC mcs_util_command_line
  PUBLIC mcs_util
  PUBLIC mcs_util_FMT
  PUBLIC mcs_util_syscall
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()

add_subdirectory (src)
