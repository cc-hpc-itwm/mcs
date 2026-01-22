# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_fuse)
target_include_directories (mcs_fuse PUBLIC include)
target_link_libraries (mcs_fuse
  PUBLIC fmt
  PUBLIC fuse3
  PUBLIC mcs_config
  PUBLIC mcs_core
  PUBLIC mcs_error
  PUBLIC mcs_serialization
  PUBLIC mcs_util
  PUBLIC mcs_util_FMT
  PUBLIC mcs_util_lock
  PUBLIC mcs_util_syscall
)

# The fuse state stores inodes. The inodes are queried by their
# uint64_t unique identifier. So the ordered set must order uint64_t's
# and the unordered set must compute hash values of unit64_t's and
# compare uint64_t's for equality.
#
set (MCS_FUSE_STATE_CONTAINER "unordered_set"
  CACHE
  STRING "[ 'unordered_set' \
          | 'ordered_set'   \
          ]"
)
set_property (CACHE MCS_FUSE_STATE_CONTAINER
  PROPERTY STRINGS "unordered_set"
                   "ordered_set"
)

if (MCS_FUSE_STATE_CONTAINER STREQUAL "unordered_set")
  target_compile_definitions (mcs_fuse
    PUBLIC MCS_FUSE_STATE_CONTAINER_UNORDERED_SET
  )
endif()
if (MCS_FUSE_STATE_CONTAINER STREQUAL "ordered_set")
  target_compile_definitions (mcs_fuse
    PUBLIC MCS_FUSE_STATE_CONTAINER_ORDERED_SET
  )
endif()

# Directory inodes store name and inode of their entries. The entries
# are queried by their names, which are of type std::string. So the
# ordered set must order std::string's and the unordered set must
# compute hash values of std::string's and compare std::string's for
# equality.
#
set (MCS_FUSE_INODE_DIRECTORY_CONTAINER "unordered_set"
  CACHE
  STRING "[ 'unordered_set' \
          | 'ordered_set'   \
          ]"
)
set_property (CACHE MCS_FUSE_INODE_DIRECTORY_CONTAINER
  PROPERTY STRINGS "unordered_set"
                   "ordered_set"
)

if (MCS_FUSE_INODE_DIRECTORY_CONTAINER STREQUAL "unordered_set")
  target_compile_definitions (mcs_fuse
    PUBLIC MCS_FUSE_INODE_DIRECTORY_CONTAINER_UNORDERED_SET
  )
endif()
if (MCS_FUSE_INODE_DIRECTORY_CONTAINER STREQUAL "ordered_set")
  target_compile_definitions (mcs_fuse
    PUBLIC MCS_FUSE_INODE_DIRECTORY_CONTAINER_ORDERED_SET
  )
endif()

add_subdirectory (src)
add_subdirectory (bin)
