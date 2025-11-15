# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_serialization name)
  add_executable (mcs_test_serialization_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_serialization_${name}
    PRIVATE mcs_config
    PRIVATE mcs_serialization
    PRIVATE mcs_testing
  )
  gtest_discover_tests (mcs_test_serialization_${name})
endfunction()

mcs_test_serialization (Concepts)
mcs_test_serialization (error)
mcs_test_serialization (temporaries)
mcs_test_serialization (OArchive_append_versus_stream)

add_subdirectory (STD)
