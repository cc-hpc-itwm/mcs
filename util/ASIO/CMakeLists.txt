# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util_ASIO)

target_include_directories (mcs_util_ASIO PUBLIC include)
target_link_libraries (mcs_util_ASIO
  PUBLIC asio
  PUBLIC fmt
  PUBLIC mcs_util
  PUBLIC mcs_util_hash
  PUBLIC mcs_util_read
  PUBLIC mcs_util_string
  PUBLIC mcs_serialization
)

add_subdirectory (src)
add_subdirectory (demo)
