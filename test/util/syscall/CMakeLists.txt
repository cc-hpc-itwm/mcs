# Copyright (C) 2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_syscall name)
  add_executable (mcs_test_util_syscall_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_syscall_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util_syscall
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_util_syscall_${name})
endfunction()

mcs_test_util_syscall (fstat)
mcs_test_util_syscall (fstatat)
mcs_test_util_syscall (mkdirat)
mcs_test_util_syscall (open)
mcs_test_util_syscall (openat)
mcs_test_util_syscall (renameat)
mcs_test_util_syscall (renameat2)
mcs_test_util_syscall (unlinkat)
