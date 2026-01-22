# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_fuse_executable prefix name)
  add_executable (${prefix}_${name}
    ${name}.cpp
  )
  target_link_libraries (${prefix}_${name}
    PRIVATE fmt
    PRIVATE mcs_config
    PRIVATE mcs_fuse
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    PRIVATE mcs_util
    PRIVATE mcs_util_syscall
    ${ARGN}
  )
endfunction()

function (mcs_test_fuse name)
  mcs_test_fuse_executable (mcs_test_fuse ${name})
  gtest_discover_tests (mcs_test_fuse_${name}
    PROPERTIES LABELS "MCSFuse"
  )
endfunction()

mcs_test_fuse (basics)

function (mcs_test_fuse_performance name)
  mcs_test_fuse_executable (mcs_test_fuse_performance ${name})
  gtest_discover_tests (mcs_test_fuse_performance_${name}
    PROPERTIES LABELS "MCSFuse_performance"
  )
endfunction()

mcs_test_fuse_performance (create_remove_write)

add_subdirectory (bin)
