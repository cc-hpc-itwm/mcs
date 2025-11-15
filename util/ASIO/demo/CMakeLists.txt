# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_executable (mcs_util_ASIO_demo_provider provider.cpp)
add_executable (mcs_util_ASIO_demo_client client.cpp)
target_link_libraries (mcs_util_ASIO_demo_provider
  PRIVATE asio
  PRIVATE mcs_util
  PRIVATE mcs_util_read
  PRIVATE mcs_util_syscall
  PRIVATE fmt
)
target_link_libraries (mcs_util_ASIO_demo_client
  PRIVATE asio
  PRIVATE mcs_util
  PRIVATE fmt
)
