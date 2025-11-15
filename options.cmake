# Copyright (C) 2024-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (MCS_CXX_STANDARD "20" CACHE STRING "['20' | '23']")
set_property (CACHE MCS_CXX_STANDARD PROPERTY STRINGS "20" "23")

option (MCS_TEST "build tests and the internal testing utilities" OFF)

option (MCS_INSTALL "install the MCS" ON)

option (MCS_IOV_BACKEND "compile with iov_backend" OFF)
