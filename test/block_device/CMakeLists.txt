# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_executable (mcs_test_block_device_basics
  basics.cpp
)
target_link_libraries (mcs_test_block_device_basics
  PRIVATE mcs_config
  PRIVATE mcs_block_device
  PRIVATE mcs_core
  PRIVATE mcs_testing
  PRIVATE mcs_testing_core
  PRIVATE mcs_testing_block_device
  PRIVATE mcs_testing_RPC
)
gtest_discover_tests (mcs_test_block_device_basics)

add_subdirectory (bin)
