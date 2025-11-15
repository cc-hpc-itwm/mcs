# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_read name)
  add_executable (mcs_test_util_read_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_read_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    PRIVATE mcs_util_read
    ${ARGSN}
  )
  gtest_discover_tests (mcs_test_util_read_${name})
endfunction()

mcs_test_util_read (State)
mcs_test_util_read (intrusive PRIVATE mcs_util_FMT)
mcs_test_util_read (one_of)
mcs_test_util_read (prefix)
mcs_test_util_read (string PRIVATE mcs_util_string)
mcs_test_util_read (uint)

add_subdirectory (STD)
