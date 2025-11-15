# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_util_ASIO
  PRIVATE Connectable.cpp
  PRIVATE SetSocketOptions.cpp
)
target_link_libraries (mcs_util_ASIO
  PRIVATE mcs_util_string
  PRIVATE mcs_serialization
  PRIVATE mcs_util_syscall
)
set_target_properties (mcs_util_ASIO PROPERTIES POSITION_INDEPENDENT_CODE ON)
