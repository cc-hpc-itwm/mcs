# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_testing_core_storage_implement_c_api name)
  add_library (mcs_testing_core_storage_implement_c_api_${name} SHARED)
  target_sources (mcs_testing_core_storage_implement_c_api_${name}
    PRIVATE "${name}.cpp"
  )
  target_link_libraries (mcs_testing_core_storage_implement_c_api_${name}
    PRIVATE mcs_core_storage_c_api
    PRIVATE mcs_core
  )
endfunction()

mcs_testing_core_storage_implement_c_api (Files)
mcs_testing_core_storage_implement_c_api (Heap)
mcs_testing_core_storage_implement_c_api (SHMEM)
