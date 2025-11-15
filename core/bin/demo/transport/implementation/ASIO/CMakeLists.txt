# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_executable (mcs_core_bin_demo_transport_implementation_ASIO_provider
  provider.cpp
)
target_link_libraries (mcs_core_bin_demo_transport_implementation_ASIO_provider
  PRIVATE fmt
  PRIVATE mcs_core
  PRIVATE mcs_rpc
  PRIVATE mcs_util
  PRIVATE mcs_util_read
  PRIVATE mcs_util_syscall
)

add_executable (mcs_core_bin_demo_transport_implementation_ASIO_client
  client.cpp
)
target_link_libraries (mcs_core_bin_demo_transport_implementation_ASIO_client
  PRIVATE fmt
  PRIVATE mcs_core
  PRIVATE mcs_nonstd_scope
  PRIVATE mcs_rpc
  PRIVATE mcs_util
  PRIVATE mcs_util_FMT
  PRIVATE mcs_util_read
)

if (MCS_INSTALL)
  install (TARGETS
    mcs_core_bin_demo_transport_implementation_ASIO_provider
    mcs_core_bin_demo_transport_implementation_ASIO_client
    RUNTIME
  )
endif()
