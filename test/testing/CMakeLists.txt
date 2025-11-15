# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_testing name)
  add_executable (mcs_test_testing_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_testing_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
  )
  gtest_discover_tests (mcs_test_testing_${name})
endfunction()

mcs_test_testing (UniqID)
mcs_test_testing (UniqTemporaryDirectory)
mcs_test_testing (require_exception)
mcs_test_testing (until)

add_subdirectory (random)
