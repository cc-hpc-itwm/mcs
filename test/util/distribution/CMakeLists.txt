# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_distribution name)
  add_executable (mcs_test_util_distribution_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_distribution_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_util_distribution_${name})
endfunction()

mcs_test_util_distribution (equal)
