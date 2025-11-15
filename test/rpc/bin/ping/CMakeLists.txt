# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (TEST_RPC_BIN_PING_SERVER "${CMAKE_BINARY_DIR}/rpc/bin/ping/mcs_rpc_bin_ping_server")
set (TEST_RPC_BIN_PING_CLIENT "${CMAKE_BINARY_DIR}/rpc/bin/ping/mcs_rpc_bin_ping_client")
set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_rpc_bin_ping
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.sh"
)
