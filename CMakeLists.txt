# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

cmake_minimum_required (VERSION 3.24)
project (mcs VERSION 0.0.1 LANGUAGES CXX C)

include ("${CMAKE_CURRENT_LIST_DIR}/options.cmake")

add_subdirectory (3rdparty/asio)
add_subdirectory (3rdparty/fmt)
add_subdirectory (block_device)
add_subdirectory (config)
add_subdirectory (core)
add_subdirectory (error)
add_subdirectory (nonstd)
add_subdirectory (rpc)
add_subdirectory (serialization)
add_subdirectory (share_service)
add_subdirectory (util)

if (MCS_IOV_BACKEND)
  add_subdirectory (3rdparty/libiov) # todo: replace with find_package
  add_subdirectory (iov_backend)
endif()

if (MCS_TEST)
  enable_testing()
  add_subdirectory (3rdparty/gtest)
  add_subdirectory (testing)
  add_subdirectory (test)
endif()
