# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_testing_iov_backend
  PRIVATE IOV_Backend.cpp
)
target_link_libraries (mcs_testing_iov_backend
  PRIVATE fmt
  PRIVATE mcs_testing
  PRIVATE mcs_util
  PRIVATE mcs_util_FMT
)
