# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_executable (mcs_fuse_bin_fs fs.cpp)
target_link_libraries (mcs_fuse_bin_fs
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_fuse
  PRIVATE mcs_util_FMT
  PRIVATE mcs_util_type
)
if (MCS_INSTALL)
  install (TARGETS mcs_fuse_bin_fs RUNTIME)
endif()

add_subdirectory (storage)
