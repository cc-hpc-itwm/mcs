# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_HeterogeneousMap INTERFACE)
target_include_directories (mcs_util_HeterogeneousMap INTERFACE include)
target_link_libraries (mcs_util_HeterogeneousMap
  INTERFACE mcs_config
  INTERFACE mcs_error
  INTERFACE mcs_util_type
  INTERFACE fmt
  INTERFACE mcs_nonstd_scope
)
