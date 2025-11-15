# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_type INTERFACE)
target_include_directories (mcs_util_type INTERFACE include)
target_link_libraries (mcs_util_type
  INTERFACE mcs_config
  INTERFACE mcs_serialization
  INTERFACE mcs_util
  INTERFACE mcs_util_read
)
