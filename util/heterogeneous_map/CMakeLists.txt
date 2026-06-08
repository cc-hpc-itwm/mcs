# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_HeterogeneousMap INTERFACE)
target_include_directories (mcs_util_HeterogeneousMap INTERFACE include)
target_link_libraries (mcs_util_HeterogeneousMap
  INTERFACE fmt
  INTERFACE mcs_config
  INTERFACE mcs_error
  INTERFACE mcs_nonstd_scope
  INTERFACE mcs_util_concurrency
  INTERFACE mcs_util_type
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()
