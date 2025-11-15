# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_hash INTERFACE)
target_include_directories (mcs_util_hash INTERFACE include)
target_link_libraries (mcs_util_hash
  INTERFACE mcs_config
  INTERFACE mcs_util
)
