# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_executable (mcs_test_core_share_between_processes_reverse_ints
  reverse_ints.cpp
)
target_link_libraries (mcs_test_core_share_between_processes_reverse_ints
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)
add_executable (mcs_test_core_share_between_processes_test
  test.cpp
)
target_link_libraries (mcs_test_core_share_between_processes_test
  PRIVATE asio
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_rpc
  PRIVATE mcs_testing
  PRIVATE mcs_testing_core
  PRIVATE mcs_testing_RPC
  PRIVATE mcs_util
)
target_compile_definitions (mcs_test_core_share_between_processes_test
  PRIVATE -DMCS_CORE_TEST_SHARE_BETWEEN_PROCESSES_REVERSE_INTS="$<TARGET_FILE:mcs_test_core_share_between_processes_reverse_ints>"
)
gtest_discover_tests (mcs_test_core_share_between_processes_test)
