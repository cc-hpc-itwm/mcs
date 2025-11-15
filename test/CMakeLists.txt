# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

find_package (gtest REQUIRED)
include (GoogleTest)

add_subdirectory (block_device)
add_subdirectory (core)
add_subdirectory (nonstd)
add_subdirectory (rpc)
add_subdirectory (serialization)
add_subdirectory (share_service)
add_subdirectory (testing)
add_subdirectory (util)

if (MCS_IOV_BACKEND)
  add_subdirectory (iov_backend)
endif()
