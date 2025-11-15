# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_iov_backend_performance _name)
  add_executable (mcs_test_iov_backend_performance_${_name}
    ${_name}.cpp
  )
  target_link_libraries (mcs_test_iov_backend_performance_${_name}
    PRIVATE mcs_config
    PRIVATE mcs_iov_backend
    PRIVATE iov::iov
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    PRIVATE mcs_testing_iov_backend
    ${ARGSN}
  )
  gtest_discover_tests (mcs_test_iov_backend_performance_${_name}
    PROPERTIES LABELS "performance"
  )
endfunction()

mcs_test_iov_backend_performance (collection_management)
mcs_test_iov_backend_performance (multiple_producers_and_multiple_consumers
  PRIVATE mcs_util_FMT
)
