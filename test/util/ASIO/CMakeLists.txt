# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_util_ASIO name)
  add_executable (mcs_test_util_ASIO_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_util_ASIO_${name}
    PRIVATE mcs_config
    PRIVATE mcs_testing
    PRIVATE mcs_util
    PRIVATE mcs_util_ASIO
  )
  gtest_discover_tests (mcs_test_util_ASIO_${name})
endfunction()

mcs_test_util_ASIO (Endpoint)
