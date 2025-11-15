# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_string)

target_include_directories (mcs_util_string PUBLIC include)
target_link_libraries (mcs_util_string
  PUBLIC mcs_serialization
  PUBLIC mcs_util
  PUBLIC mcs_util_read
  PUBLIC mcs_util_hash
)

add_subdirectory (src)
