# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_rpc_bin_ping_Dispatcher
  Dispatcher.cpp
)
target_link_libraries (mcs_rpc_bin_ping_Dispatcher
  PRIVATE mcs_config
  PUBLIC mcs_serialization
  PUBLIC mcs_rpc
  PRIVATE mcs_util_tuplish
)

add_executable (mcs_rpc_bin_ping_server
  server.cpp
)
target_link_libraries (mcs_rpc_bin_ping_server
  PRIVATE mcs_config
  PRIVATE fmt
  PRIVATE mcs_rpc
  PRIVATE mcs_rpc_bin_ping_Dispatcher
  PRIVATE mcs_util
  PRIVATE mcs_util_syscall
  PRIVATE mcs_util_read
)

add_executable (mcs_rpc_bin_ping_client
  client.cpp
)
target_link_libraries (mcs_rpc_bin_ping_client
  PRIVATE mcs_config
  PRIVATE fmt
  PRIVATE mcs_rpc
  PRIVATE mcs_rpc_bin_ping_Dispatcher
  PRIVATE mcs_util
  PRIVATE mcs_util_syscall
  PRIVATE mcs_util_read
)

if (MCS_INSTALL)
  install (TARGETS
    mcs_rpc_bin_ping_server
    mcs_rpc_bin_ping_client
    RUNTIME
  )
endif()
