# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_type name)
  add_executable (mcs_test_util_type_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_type_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util_type
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_util_type_${name})
endfunction()

mcs_test_util_type (At)
mcs_test_util_type (Size)
mcs_test_util_type (ID)
mcs_test_util_type (Index)
mcs_test_util_type (List)
mcs_test_util_type (Tag)
