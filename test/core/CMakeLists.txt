# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (manage_storage_in_remote_process)
add_subdirectory (manage_segments_in_remote_process)
add_subdirectory (memory)
add_subdirectory (share_between_processes)
add_subdirectory (storage)
add_subdirectory (transport)

function (mcs_test_core name)
  add_executable (mcs_test_core_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_core_${name}
    PRIVATE mcs_config
    PRIVATE mcs_core
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    ${ARGN}
  )
  gtest_discover_tests (mcs_test_core_${name})
endfunction()

mcs_test_core (segment_create)
mcs_test_core (share_in_the_same_process
  PRIVATE mcs_testing_RPC
  PRIVATE mcs_util
)
mcs_test_core (storage_management)

add_subdirectory (bin/demo/transport/implementation/ASIO)
