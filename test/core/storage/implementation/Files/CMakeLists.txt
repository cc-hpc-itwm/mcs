# Copyright (C) 2024-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_core_storage_implementation_Files name)
  add_executable (mcs_test_core_storage_implementation_Files_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_core_storage_implementation_Files_${name}
    PRIVATE mcs_config
    PRIVATE mcs_core
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    PRIVATE mcs_util_FMT
    PRIVATE mcs_util_read
  )
  gtest_discover_tests (mcs_test_core_storage_implementation_Files_${name})
endfunction()

mcs_test_core_storage_implementation_Files (PrefixContainsNonSegmentFile)
