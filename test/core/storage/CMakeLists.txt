# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_core_storage name)
  add_executable (mcs_test_core_storage_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_core_storage_${name}
    PRIVATE mcs_config
    PRIVATE mcs_core
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
  )
  gtest_discover_tests (mcs_test_core_storage_${name})
endfunction()

mcs_test_core_storage (MaxSize)

add_subdirectory (implementation)
