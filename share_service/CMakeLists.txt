# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (bin)

add_library (mcs_share_service)
target_include_directories (mcs_share_service PUBLIC include)
target_link_libraries (mcs_share_service
  PUBLIC asio
  PUBLIC mcs_core
  PUBLIC mcs_serialization
  PUBLIC mcs_util_ASIO
)

add_library (mcs_share_service_tcp_provider)
target_include_directories (mcs_share_service_tcp_provider PUBLIC include)

add_library (mcs_share_service_shmem_client)
target_include_directories (mcs_share_service_shmem_client PUBLIC include)

add_subdirectory (src)
