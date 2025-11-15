# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_serialization_implementation_STD_filesystem name)
  add_executable (mcs_test_serialization_implementation_STD_filesystem_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_serialization_implementation_STD_filesystem_${name}
    PRIVATE mcs_config
    PRIVATE mcs_serialization
    PRIVATE mcs_testing
  )
  gtest_discover_tests (mcs_test_serialization_implementation_STD_filesystem_${name})
endfunction()

mcs_test_serialization_implementation_STD_filesystem (path)
