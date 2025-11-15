# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_iov_backend_provider_distribution _name)
  add_executable (mcs_test_iov_backend_provider_distribution_${_name}
    ${_name}.cpp
  )
  target_link_libraries (mcs_test_iov_backend_provider_distribution_${_name}
    PRIVATE mcs_config
    PRIVATE mcs_iov_backend
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    ${ARGSN}
  )
  gtest_discover_tests (mcs_test_iov_backend_provider_distribution_${_name})
endfunction()

mcs_test_iov_backend_provider_distribution (AsEqualAsPossible)
