# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_share_service
  PRIVATE Chunk.cpp
  PRIVATE command/Create.cpp
  PRIVATE command/Remove.cpp
)

target_sources (mcs_share_service_tcp_provider
  PRIVATE tcp_provider.cpp
)
target_link_libraries (mcs_share_service_tcp_provider
  PRIVATE mcs_share_service
  PRIVATE mcs_util
  PRIVATE mcs_util_syscall
  PRIVATE mcs_util_FMT
  PRIVATE asio
)

target_sources (mcs_share_service_shmem_client
  PRIVATE shmem_client.cpp
)
target_link_libraries (mcs_share_service_shmem_client
  PRIVATE asio
  PRIVATE fmt
  PRIVATE mcs_share_service
  PRIVATE mcs_rpc
  PRIVATE mcs_util
)
