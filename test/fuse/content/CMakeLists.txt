# Copyright (C) 2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

mcs_test_fuse_executable (mcs_test_fuse_content select)
gtest_discover_tests (mcs_test_fuse_content_select
  PROPERTIES LABELS "MCSFuse"
)
