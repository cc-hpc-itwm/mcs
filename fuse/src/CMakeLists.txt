# Copyright (C) 2025-2026 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_fuse
  PRIVATE content/implementation/Realloc.cpp
  PRIVATE content/select.cpp
  PRIVATE fuse/reply.cpp
  PRIVATE session/Options.cpp
  PRIVATE state/inode/Common.cpp
)
target_link_libraries (mcs_fuse
  PRIVATE mcs_util_syscall
)
set_target_properties (mcs_fuse
  PROPERTIES POSITION_INDEPENDENT_CODE ON
)
