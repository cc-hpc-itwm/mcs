# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_serialization_STD name)
  add_executable (mcs_test_serialization_STD_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_serialization_STD_${name}
    PRIVATE mcs_config
    PRIVATE mcs_serialization
    PRIVATE mcs_testing
  )
  gtest_discover_tests (mcs_test_serialization_STD_${name})
endfunction()

mcs_test_serialization_STD (List)
mcs_test_serialization_STD (Map)
mcs_test_serialization_STD (Optional)
mcs_test_serialization_STD (Set)
mcs_test_serialization_STD (String)
mcs_test_serialization_STD (Tuple)
mcs_test_serialization_STD (UnorderedMap)
mcs_test_serialization_STD (Variant)
mcs_test_serialization_STD (Vector)

add_subdirectory (filesystem)
