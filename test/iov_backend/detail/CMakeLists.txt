# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_test_iov_backend_detail name)
  add_executable (mcs_test_iov_backend_detail_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_test_iov_backend_detail_${name}
    PRIVATE mcs_config
    PRIVATE mcs_iov_backend
    PRIVATE mcs_testing
  )
  gtest_discover_tests (mcs_test_iov_backend_detail_${name})
endfunction()

mcs_test_iov_backend (Bitmap)
mcs_test_iov_backend (Queue)
