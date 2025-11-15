# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_read_STD name)
  add_executable (mcs_test_util_read_STD_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_read_STD_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    PRIVATE mcs_util_string
  )
  gtest_discover_tests (mcs_test_util_read_STD_${name})
endfunction()

mcs_test_util_read_STD (list)
mcs_test_util_read_STD (optional PRIVATE mcs_util_FMT)
mcs_test_util_read_STD (tuple)
mcs_test_util_read_STD (variant PRIVATE mcs_util_FMT)
mcs_test_util_read_STD (vector)
