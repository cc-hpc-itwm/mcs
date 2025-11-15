# Copyright (C) 2022,2024-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_serialization)
target_include_directories (mcs_serialization PUBLIC include)
target_link_libraries (mcs_serialization
  PUBLIC mcs_config
  PUBLIC mcs_util
  PUBLIC mcs_util_read
)
add_subdirectory (src)
