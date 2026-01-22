# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (MCS_TEST_FUSE_BIN_STORAGE_FS "${CMAKE_BINARY_DIR}/fuse/bin/storage/mcs_fuse_bin_storage_fs")
set (MCS_TEST_FUSE_BIN_STORAGE_DEMO_PRODUCER "${CMAKE_BINARY_DIR}/fuse/bin/storage/demo/mcs_fuse_bin_storage_demo_producer")
set (MCS_TEST_FUSE_BIN_STORAGE_DEMO_CONSUMER "${CMAKE_BINARY_DIR}/fuse/bin/storage/demo/mcs_fuse_bin_storage_demo_consumer")
set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_fuse_bin_storage_demo
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.sh"
)
set_property (TEST mcs_fuse_bin_storage_demo APPEND PROPERTY LABELS "MCSFuse")
