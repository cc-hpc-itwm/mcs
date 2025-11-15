# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_core_storage_implementation name)
  add_executable (mcs_test_core_storage_implementation_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_core_storage_implementation_${name}
    PRIVATE mcs_config
    PRIVATE mcs_core
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_core_storage_implementation_${name})
endfunction()

mcs_test_core_storage_implementation (Files)
mcs_test_core_storage_implementation (Heap)
mcs_test_core_storage_implementation (Import_C_API)
mcs_test_core_storage_implementation (SHMEM)
mcs_test_core_storage_implementation (Trace
  PRIVATE mcs_util_FMT
  PRIVATE mcs_util_read
)
mcs_test_core_storage_implementation (TraceWithCustomTracer
  PRIVATE mcs_util_FMT
)
mcs_test_core_storage_implementation (Virtual PRIVATE mcs_util_syscall)
target_compile_definitions (mcs_test_core_storage_implementation_Virtual
  PRIVATE -DMCS_TEST_CORE_STORAGE_IMPLEMENTATION_VIRTUAL_CHUNKY=\"$<TARGET_FILE:mcs_test_core_storage_implementation_chunky>\"
)
add_dependencies (mcs_test_core_storage_implementation_Virtual
  mcs_test_core_storage_implementation_chunky
)
mcs_test_core_storage_implementation (more_segments_than_max_open_files_is_possible
  PRIVATE mcs_util_syscall
)

add_subdirectory (Files)
add_subdirectory (virtual)
