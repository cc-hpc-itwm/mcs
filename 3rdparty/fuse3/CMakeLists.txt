# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

find_package (PkgConfig REQUIRED)
pkg_check_modules (fuse3 REQUIRED fuse3=3.18.1)

add_library (fuse3 INTERFACE)
target_include_directories (fuse3 INTERFACE ${fuse3_INCLUDE_DIRS})
target_link_libraries (fuse3 INTERFACE ${fuse3_LINK_LIBRARIES})
