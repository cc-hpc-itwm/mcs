# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (ASIO)

if (MCS_TRANSPORT_LIBFABRIC)
  add_subdirectory (libfabric)

  function (mcs_test_core_transport_implementation name)
    add_executable (mcs_test_core_transport_implementation_${name}
      ${name}.cpp
    )
    target_link_libraries (mcs_test_core_transport_implementation_${name}
      PRIVATE libfabric
      PRIVATE mcs_config
      PRIVATE mcs_core
      PRIVATE mcs_testing
      PRIVATE mcs_testing_core
      PRIVATE mcs_testing_RPC
    )
    gtest_discover_tests (mcs_test_core_transport_implementation_${name})
  endfunction()

  mcs_test_core_transport_implementation (mix_ASIO_and_libfabric)
endif()
