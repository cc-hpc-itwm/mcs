# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (bin)

add_library (mcs_block_device)
target_include_directories (mcs_block_device PUBLIC include)
target_link_libraries (mcs_block_device
  PUBLIC mcs_config
  PUBLIC fmt
  PUBLIC mcs_core
  PUBLIC mcs_error
  PUBLIC mcs_nonstd_scope
  PUBLIC mcs_rpc
  PUBLIC mcs_serialization
  PUBLIC mcs_util_tuplish
  PUBLIC mcs_util
  PUBLIC mcs_util_ASIO
)
add_subdirectory (src)
