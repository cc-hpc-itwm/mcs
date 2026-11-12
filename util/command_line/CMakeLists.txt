# Copyright (C) 2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_command_line)
target_include_directories (mcs_util_command_line PUBLIC include)
target_link_libraries (mcs_util_command_line
  PUBLIC fmt
  PUBLIC mcs_util
  PUBLIC mcs_util_FMT
  PRIVATE mcs_config
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()

add_subdirectory (src)
