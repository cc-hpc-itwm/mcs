# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_test_util_dlhandle_library SHARED)
target_sources (mcs_test_util_dlhandle_library PRIVATE "library.cpp")
target_link_libraries (mcs_test_util_dlhandle_library PRIVATE mcs_config)

function (mcs_test_util_dlhandle name)
  add_executable (mcs_test_util_dlhandle_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_dlhandle_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    PRIVATE mcs_test_util_dlhandle_library
    ${ARGN}
  )
  target_compile_definitions (mcs_test_util_dlhandle_${name}
    PRIVATE "-DMCS_TEST_UTIL_DLHANDLE_LIBRARY=\"$<TARGET_FILE:mcs_test_util_dlhandle_library>\""
  )
  gtest_discover_tests (mcs_test_util_dlhandle_${name})
endfunction()

mcs_test_util_dlhandle (test)
