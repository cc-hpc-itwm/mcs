# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_lock)
target_include_directories (mcs_util_lock PUBLIC include)
target_link_libraries (mcs_util_lock
  PUBLIC mcs_config
  PUBLIC mcs_util
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()

add_subdirectory (src)
