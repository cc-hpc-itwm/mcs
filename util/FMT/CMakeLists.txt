# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_FMT)
target_include_directories (mcs_util_FMT PUBLIC include)
target_link_libraries (mcs_util_FMT
  PUBLIC mcs_util_string
  PRIVATE mcs_config
  PRIVATE mcs_error
  PRIVATE mcs_util
  PRIVATE fmt
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()

add_subdirectory (src)
