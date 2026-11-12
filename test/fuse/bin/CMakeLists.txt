# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (MCS_TEST_FUSE_BIN_FS "${CMAKE_BINARY_DIR}/fuse/bin/mcs_fuse_bin_fs")
set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

add_subdirectory (detail)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_fuse_bin_fs
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.sh"
    "$<TARGET_FILE:mcs_test_fuse_bin_detail_VirtualArguments>"
)
set_property (TEST mcs_fuse_bin_fs APPEND PROPERTY LABELS "MCSFuse")

add_subdirectory (storage)
