# Copyright (C) 2022-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_util)
target_include_directories (mcs_util PUBLIC include)
target_link_libraries (mcs_util
  PUBLIC mcs_config
  PUBLIC mcs_error
  PUBLIC mcs_util_syscall # DLHandle only
  PUBLIC mcs_util_FMT
  PUBLIC fmt
)

if (MCS_INSTALL)
  INSTALL (DIRECTORY include/mcs DESTINATION include)
endif()

add_subdirectory (src)

add_subdirectory (ASIO)
add_subdirectory (FMT)
add_subdirectory (hash)
add_subdirectory (heterogeneous_map)
add_subdirectory (read)
add_subdirectory (string)
add_subdirectory (syscall)
add_subdirectory (tuplish)
add_subdirectory (type)

# \todo further split "mcs_util" into multiple independent targets
# with minimal external dependencies
