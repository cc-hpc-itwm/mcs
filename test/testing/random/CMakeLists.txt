# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_testing_random name)
  add_executable (mcs_test_testing_random_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_testing_random_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
  )
  gtest_discover_tests (mcs_test_testing_random_${name})
endfunction()

mcs_test_testing_random (one_of)
