# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_FMT INTERFACE)
target_include_directories (mcs_util_FMT INTERFACE include)
target_link_libraries (mcs_util_FMT
  INTERFACE mcs_config
  INTERFACE mcs_error
  INTERFACE mcs_util
  INTERFACE mcs_util_string
  INTERFACE fmt
)
