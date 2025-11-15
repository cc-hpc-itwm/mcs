# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_test_core_storage_implementation_chunky SHARED)
target_sources (mcs_test_core_storage_implementation_chunky
  PRIVATE "chunky.c"
)
target_link_libraries (mcs_test_core_storage_implementation_chunky
  PRIVATE mcs_core_storage_c_api
)
