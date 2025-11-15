# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_core_transport_implementation_ASIO name)
  add_executable (mcs_test_core_transport_implementation_ASIO_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_core_transport_implementation_ASIO_${name}
    PRIVATE mcs_config
    PRIVATE mcs_core
    PRIVATE mcs_testing
    PRIVATE mcs_testing_core
    PRIVATE mcs_testing_RPC
  )
  gtest_discover_tests (mcs_test_core_transport_implementation_ASIO_${name})
endfunction()

mcs_test_core_transport_implementation_ASIO (memory_get_put_works)
