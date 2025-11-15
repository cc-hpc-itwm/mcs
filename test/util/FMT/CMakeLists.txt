# Copyright (C) 2024-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_FMT name)
  add_executable (mcs_test_util_FMT_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_FMT_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    PRIVATE mcs_util_FMT
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_util_FMT_${name})
endfunction()

mcs_test_util_FMT (Concepts)
mcs_test_util_FMT (WriteFileFailed)
