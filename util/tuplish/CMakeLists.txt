# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_tuplish INTERFACE)
target_include_directories (mcs_util_tuplish
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries (mcs_util_tuplish
  INTERFACE mcs_config
  INTERFACE mcs_util
  INTERFACE mcs_util_read
  INTERFACE mcs_serialization
)
