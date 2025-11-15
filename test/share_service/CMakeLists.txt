# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (MCS_TEST_SHARE_SERVICE_BIN_DIR "${PROJECT_BINARY_DIR}/share_service/bin")
set (MCS_TEST_SHARE_SERVICE_BIN_PROVIDER "${MCS_TEST_SHARE_SERVICE_BIN_DIR}/mcs_share_service_bin_provider")
set (MCS_TEST_SHARE_SERVICE_BIN_TCP_PROVIDER "${MCS_TEST_SHARE_SERVICE_BIN_DIR}/mcs_share_service_bin_tcp_provider")
set (MCS_TEST_SHARE_SERVICE_BIN_CREATE "${MCS_TEST_SHARE_SERVICE_BIN_DIR}/mcs_share_service_bin_create")
set (MCS_TEST_SHARE_SERVICE_BIN_REMOVE "${MCS_TEST_SHARE_SERVICE_BIN_DIR}/mcs_share_service_bin_remove")
set (MCS_TEST_SHARE_SERVICE_BIN_CAT "${MCS_TEST_SHARE_SERVICE_BIN_DIR}/mcs_share_service_bin_cat")
set (MCS_TEST_SHARE_SERVICE_BIN_WRITE "${MCS_TEST_SHARE_SERVICE_BIN_DIR}/mcs_share_service_bin_write")
set (MCS_TEST_SHARE_SERVICE_TESTCASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.generic.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.generic.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_share_service_test_generic_create_cat_write_remove
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.generic.sh" create_cat_write_remove
)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.tcp_provider.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.tcp_provider.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_share_service_test_tcp_provider_create_cat_write_remove
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.tcp_provider.sh" create_cat_write_remove
)

add_executable (mcs_test_share_service_shmem_client
  shmem_client.cpp
)
target_link_libraries (mcs_test_share_service_shmem_client
  PRIVATE mcs_config
  PRIVATE mcs_testing
  PRIVATE mcs_testing_RPC
  PRIVATE mcs_share_service
  PRIVATE mcs_share_service_shmem_client
)
gtest_discover_tests (mcs_test_share_service_shmem_client)
