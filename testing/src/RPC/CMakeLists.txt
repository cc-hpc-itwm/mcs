# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

include ("${CMAKE_CURRENT_LIST_DIR}/options.cmake")

add_library (mcs_testing_RPC
  ProtocolState.cpp
)
target_include_directories (mcs_testing_RPC
  PUBLIC ${PROJECT_SOURCE_DIR}/../include
)
target_link_libraries (mcs_testing_RPC
  PRIVATE mcs_config
  PUBLIC mcs_rpc
  PUBLIC mcs_testing
)
target_compile_definitions (mcs_testing_RPC
  PRIVATE MCS_TESTING_RPC_IP_VERSION=${MCS_TESTING_RPC_IP_VERSION}
)
