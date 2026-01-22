# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_fuse_bin_storage_demo_data STATIC data.cpp)
target_link_libraries (mcs_fuse_bin_storage_demo_data
  PUBLIC mcs_config
  PRIVATE fmt
)

# Producer: A legacy application that writes data using standard file I/O.
# It represents an unmodifiable application that cannot use MCS directly.
#
add_executable (mcs_fuse_bin_storage_demo_producer producer.cpp)
target_link_libraries (mcs_fuse_bin_storage_demo_producer
  PRIVATE fmt
  PRIVATE mcs_fuse_bin_storage_demo_data
  PRIVATE mcs_util
)
if (MCS_INSTALL)
  install (TARGETS mcs_fuse_bin_storage_demo_producer RUNTIME)
endif()

# Consumer: An MCS-aware application that reads data by directly
# accessing the SHMEM storage (zero-copy).
#
add_executable (mcs_fuse_bin_storage_demo_consumer consumer.cpp)
target_link_libraries (mcs_fuse_bin_storage_demo_consumer
  PRIVATE fmt
  PRIVATE mcs_config
  PRIVATE mcs_core
  PRIVATE mcs_fuse
  PRIVATE mcs_fuse_bin_storage_demo_data
  PRIVATE mcs_util
)
if (MCS_INSTALL)
  install (TARGETS mcs_fuse_bin_storage_demo_consumer RUNTIME)
endif()
