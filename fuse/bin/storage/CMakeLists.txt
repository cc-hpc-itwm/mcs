# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_subdirectory (demo)

add_executable (mcs_fuse_bin_storage_fs fs.cpp)
target_link_libraries (mcs_fuse_bin_storage_fs
  PRIVATE fmt
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_fuse
  PRIVATE mcs_util_FMT
  PRIVATE mcs_util_read
  PRIVATE mcs_util_type
)
if (MCS_INSTALL)
  install (TARGETS mcs_fuse_bin_storage_fs RUNTIME)
endif()

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
