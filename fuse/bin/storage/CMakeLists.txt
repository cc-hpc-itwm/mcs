# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (demo)

add_executable (mcs_fuse_bin_storage_fs_distribution distribution.cpp)
target_link_libraries (mcs_fuse_bin_storage_fs_distribution
  PRIVATE fmt
  PRIVATE mcs_config
  PRIVATE mcs_fuse
  PRIVATE mcs_util
)
if (MCS_INSTALL)
  install (TARGETS mcs_fuse_bin_storage_fs_distribution RUNTIME)
endif()
