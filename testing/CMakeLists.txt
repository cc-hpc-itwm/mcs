# Copyright (C) 2022,2024-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_testing)
target_include_directories (mcs_testing PUBLIC include)
target_link_libraries (mcs_testing
  PUBLIC GTest::gtest_main
  PUBLIC mcs_config
  PUBLIC fmt
  PUBLIC gmock
  PUBLIC mcs_util
  PUBLIC mcs_util_syscall
  PUBLIC mcs_util_FMT
  PUBLIC mcs_util_read
  PUBLIC mcs_util_string
)

add_library (mcs_testing_syscall)
target_include_directories (mcs_testing_syscall PUBLIC include)
target_link_libraries (mcs_testing_syscall
  PUBLIC mcs_config
  PUBLIC mcs_error
  PUBLIC mcs_util_syscall
  PUBLIC fmt
)

add_subdirectory (src)
