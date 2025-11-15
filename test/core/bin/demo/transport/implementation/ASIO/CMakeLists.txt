# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (MCS_TEST_CORE_BIN_DEMO_TRANSPORT_IMPLEMENTATION_ASIO_PROVIDER "${CMAKE_BINARY_DIR}/core/bin/demo/transport/implementation/ASIO/mcs_core_bin_demo_transport_implementation_ASIO_provider")
set (MCS_TEST_CORE_BIN_DEMO_TRANSPORT_IMPLEMENTATION_ASIO_CLIENT "${CMAKE_BINARY_DIR}/core/bin/demo/transport/implementation/ASIO/mcs_core_bin_demo_transport_implementation_ASIO_client")
set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_core_bin_demo_transport_ASIO_tcp
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.sh" tcp
)
add_test (
  NAME mcs_core_bin_demo_transport_ASIO_local_stream
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.sh" local_stream
)
