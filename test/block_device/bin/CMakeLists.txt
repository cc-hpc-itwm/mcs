# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

set (MCS_TEST_BLOCK_DEVICE_BIN_DIR "${PROJECT_BINARY_DIR}/block_device/bin")

set (MCS_TEST_BLOCK_DEVICE_BIN_BLOCKS "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_blocks")
set (MCS_TEST_BLOCK_DEVICE_BIN_BLOCK_SIZE "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_block_size")
set (MCS_TEST_BLOCK_DEVICE_BIN_CAT "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_cat")
set (MCS_TEST_BLOCK_DEVICE_BIN_META_DATA_PROVIDER "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_meta_data_provider")
set (MCS_TEST_BLOCK_DEVICE_BIN_NUMBER_OF_BLOCKS "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_number_of_blocks")
set (MCS_TEST_BLOCK_DEVICE_BIN_REMOVE "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_remove")
set (MCS_TEST_BLOCK_DEVICE_BIN_STORAGE_PROVIDER "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_storage_provider")
set (MCS_TEST_BLOCK_DEVICE_BIN_WRITE "${MCS_TEST_BLOCK_DEVICE_BIN_DIR}/mcs_block_device_bin_write")

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/test.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/test.sh
  FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
  @ONLY
)

add_test (
  NAME mcs_block_device_bin_test
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/test.sh"
)
