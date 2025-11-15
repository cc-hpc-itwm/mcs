# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_core_manage_segments_in_remote_process name)
endfunction()

add_executable (mcs_test_core_manage_segments_in_remote_process_segment_create
  segment_create.cpp
)
target_link_libraries (mcs_test_core_manage_segments_in_remote_process_segment_create
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_testing_core
  PRIVATE mcs_util
  PRIVATE mcs_util_FMT
  PRIVATE mcs_util_read
)

add_executable (mcs_test_core_manage_segments_in_remote_process_segment_remove
  segment_remove.cpp
)
target_link_libraries (mcs_test_core_manage_segments_in_remote_process_segment_remove
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)

add_executable (mcs_test_core_manage_segments_in_remote_process_test
  test.cpp
)
target_link_libraries (mcs_test_core_manage_segments_in_remote_process_test
  PRIVATE asio
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_testing
  PRIVATE mcs_testing_core
  PRIVATE mcs_testing_RPC
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)
target_compile_definitions (mcs_test_core_manage_segments_in_remote_process_test
  PRIVATE -DMCS_TEST_CORE_TEST_MANAGE_SEGMENTS_IN_REMOTE_PROCESS_SEGMENT_CREATE="$<TARGET_FILE:mcs_test_core_manage_segments_in_remote_process_segment_create>"
  PRIVATE -DMCS_TEST_CORE_TEST_MANAGE_SEGMENTS_IN_REMOTE_PROCESS_SEGMENT_REMOVE="$<TARGET_FILE:mcs_test_core_manage_segments_in_remote_process_segment_remove>"
)
gtest_discover_tests (mcs_test_core_manage_segments_in_remote_process_test)
