# Copyright (C) 2022,2024-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (bin)

add_library (mcs_rpc)
target_include_directories (mcs_rpc PUBLIC include)
target_link_libraries (mcs_rpc
  PUBLIC mcs_config
  PUBLIC asio
  PUBLIC mcs_nonstd_scope
  PUBLIC mcs_serialization
  PUBLIC mcs_util
  PUBLIC mcs_util_hash
  PUBLIC mcs_util_read
  PUBLIC mcs_util_ASIO
  PUBLIC mcs_util_FMT
)
add_subdirectory (src)
