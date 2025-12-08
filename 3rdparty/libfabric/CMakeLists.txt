# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

find_package (PkgConfig REQUIRED)
pkg_check_modules (LIBFABRIC REQUIRED IMPORTED_TARGET libfabric)

add_library (libfabric INTERFACE)
target_link_libraries (libfabric INTERFACE PkgConfig::LIBFABRIC)
