# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_nonstd_scope INTERFACE)
target_include_directories (mcs_nonstd_scope INTERFACE include)
target_link_libraries (mcs_nonstd_scope
  INTERFACE mcs_config
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()
