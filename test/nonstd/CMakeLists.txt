# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_executable (mcs_test_nonstd_scope
  scope.cpp
)
target_link_libraries (mcs_test_nonstd_scope
  PRIVATE mcs_config
  PRIVATE mcs_nonstd_scope
  PRIVATE mcs_testing
)
gtest_discover_tests (mcs_test_nonstd_scope)
