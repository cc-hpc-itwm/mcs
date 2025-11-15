# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_iov_backend _name)
  add_executable (mcs_test_iov_backend_${_name}
    ${_name}.cpp
  )
  target_link_libraries (mcs_test_iov_backend_${_name}
    PRIVATE mcs_config
    PRIVATE mcs_iov_backend
    PRIVATE iov::iov
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    PRIVATE mcs_testing_iov_backend
    ${ARGSN}
  )
  gtest_discover_tests (mcs_test_iov_backend_${_name})
endfunction()

mcs_test_iov_backend (Parameter)
mcs_test_iov_backend (UsedStorages)
mcs_test_iov_backend (basics
  PRIVATE mcs_util_FMT
  PRIVATE mcs_util_read
)

add_subdirectory (bin)
add_subdirectory (detail)
add_subdirectory (performance)
add_subdirectory (provider)
