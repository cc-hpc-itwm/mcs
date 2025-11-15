# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_syscall
  PRIVATE syscall.cpp
)
target_link_libraries (mcs_util_syscall
  PRIVATE fmt
  PRIVATE mcs_util
)
set_target_properties (mcs_util_syscall PROPERTIES POSITION_INDEPENDENT_CODE ON)
