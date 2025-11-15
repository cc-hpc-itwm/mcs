# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

target_sources (mcs_testing_core
  PRIVATE operator==/storage/implementation/Files/Prefix.cpp
  PRIVATE operator==/storage/implementation/SHMEM/Prefix.cpp
  PRIVATE operator==/storage/implementation/Virtual/Parameter/Create.cpp
  PRIVATE printer/memory/Offset.cpp
  PRIVATE printer/memory/Range.cpp
  PRIVATE printer/memory/Size.cpp
  PRIVATE printer/storage/MaxSize.cpp
  PRIVATE printer/storage/implementation/Files/Parameter/Create.cpp
  PRIVATE printer/storage/implementation/Files/Prefix.cpp
  PRIVATE printer/storage/implementation/Heap/Parameter/Create.cpp
  PRIVATE printer/storage/implementation/SHMEM/Parameter/Create.cpp
  PRIVATE printer/storage/implementation/SHMEM/Prefix.cpp
  PRIVATE random/memory/Offset.cpp
  PRIVATE random/memory/Range.cpp
  PRIVATE random/memory/Size.cpp
  PRIVATE random/storage/MaxSize.cpp
  PRIVATE random/storage/implementation/Files/Parameter/Create.cpp
  PRIVATE random/storage/implementation/Files/Prefix.cpp
  PRIVATE random/storage/implementation/Heap/Parameter/Create.cpp
  PRIVATE random/storage/implementation/SHMEM/Parameter/Create.cpp
  PRIVATE random/storage/implementation/SHMEM/Prefix.cpp
  PRIVATE random/storage/implementation/Virtual/Parameter/Create.cpp
  PRIVATE storage/implementation/Files.cpp
  PRIVATE storage/implementation/Heap.cpp
  PRIVATE storage/implementation/SHMEM.cpp
  PRIVATE storage/implementation/Virtual.cpp
)
target_link_libraries (mcs_testing_core
  PRIVATE fmt
)
target_compile_definitions (mcs_testing_core
  PRIVATE -DTESTING_MCS_CORE_STORAGE_IMPLEMENT_C_API_FILES="$<TARGET_FILE:mcs_testing_core_storage_implement_c_api_Files>"
  PRIVATE -DTESTING_MCS_CORE_STORAGE_IMPLEMENT_C_API_HEAP="$<TARGET_FILE:mcs_testing_core_storage_implement_c_api_Heap>"
  PRIVATE -DTESTING_MCS_CORE_STORAGE_IMPLEMENT_C_API_SHMEM="$<TARGET_FILE:mcs_testing_core_storage_implement_c_api_SHMEM>"
)
add_dependencies (mcs_testing_core
  mcs_testing_core_storage_implement_c_api_Files
  mcs_testing_core_storage_implement_c_api_Heap
  mcs_testing_core_storage_implement_c_api_SHMEM
)

add_subdirectory (storage/implementation/implement_C_API)
