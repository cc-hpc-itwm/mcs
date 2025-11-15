# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (bin)

add_library (mcs_core)
target_include_directories (mcs_core PUBLIC include)
target_link_libraries (mcs_core
  PUBLIC mcs_config
  PUBLIC asio
  PUBLIC fmt
  PUBLIC mcs_serialization
  PUBLIC mcs_rpc
  PUBLIC mcs_util_hash
  PUBLIC mcs_util_tuplish
  PUBLIC mcs_util
  PUBLIC mcs_util_ASIO
  PUBLIC mcs_util_FMT
  PUBLIC mcs_util_HeterogeneousMap
  PUBLIC mcs_util_read
  PUBLIC mcs_util_string
  PUBLIC mcs_util_syscall
  PUBLIC mcs_util_type
)

add_library (mcs_core_storage_c_api INTERFACE)
target_include_directories (mcs_core_storage_c_api INTERFACE include)

add_subdirectory (src)
