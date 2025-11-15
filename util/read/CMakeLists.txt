# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_read)

target_include_directories (mcs_util_read PUBLIC include)
target_link_libraries (mcs_util_read
  PUBLIC fmt
  PUBLIC mcs_util
  PUBLIC mcs_util_string
)

add_subdirectory (src)
