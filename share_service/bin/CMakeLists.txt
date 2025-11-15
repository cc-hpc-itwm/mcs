# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_share_service_bin INTERFACE)
target_link_libraries (mcs_share_service_bin
  INTERFACE mcs_core
  INTERFACE mcs_share_service
  INTERFACE mcs_rpc
)
target_include_directories (mcs_share_service_bin
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../../
)

add_executable (mcs_share_service_bin_provider provider.cpp)
target_link_libraries (mcs_share_service_bin_provider
  PRIVATE fmt
  PRIVATE mcs_share_service
  PRIVATE mcs_rpc
  PRIVATE mcs_util
  PRIVATE mcs_util_read
  PRIVATE mcs_util_syscall
)

add_executable (mcs_share_service_bin_create create.cpp)
target_link_libraries (mcs_share_service_bin_create
  PRIVATE fmt
  PRIVATE mcs_core
  PRIVATE mcs_share_service
  PRIVATE mcs_rpc
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)

add_executable (mcs_share_service_bin_remove remove.cpp)
target_link_libraries (mcs_share_service_bin_remove
  PRIVATE fmt
  PRIVATE mcs_core
  PRIVATE mcs_share_service
  PRIVATE mcs_share_service_bin
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)

add_executable (mcs_share_service_bin_cat cat.cpp)
target_link_libraries (mcs_share_service_bin_cat
  PRIVATE fmt
  PRIVATE mcs_core
  PRIVATE mcs_share_service
  PRIVATE mcs_share_service_bin
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)

add_executable (mcs_share_service_bin_write write.cpp)
target_link_libraries (mcs_share_service_bin_write
  PRIVATE fmt
  PRIVATE mcs_core
  PRIVATE mcs_share_service
  PRIVATE mcs_share_service_bin
  PRIVATE mcs_util
  PRIVATE mcs_util_read
)

add_executable (mcs_share_service_bin_tcp_provider tcp_provider.c)
target_link_libraries (mcs_share_service_bin_tcp_provider
  mcs_share_service_tcp_provider
)

if (MCS_INSTALL)
  install (TARGETS
    mcs_share_service_bin_provider
    mcs_share_service_bin_create
    mcs_share_service_bin_remove
    mcs_share_service_bin_cat
    mcs_share_service_bin_write
    mcs_share_service_bin_tcp_provider
    RUNTIME
  )
endif()
