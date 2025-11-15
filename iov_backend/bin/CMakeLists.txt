# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_iov_backend_bin name)
  add_executable (mcs_iov_backend_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_iov_backend_${name}
    PRIVATE fmt
    PRIVATE mcs_iov_backend
    PRIVATE mcs_rpc
    PRIVATE mcs_util
    PRIVATE mcs_util_read
    ${ARGN}
  )
  if (MCS_INSTALL)
    install (TARGETS mcs_iov_backend_${name} RUNTIME)
  endif()
endfunction()

mcs_iov_backend_bin (collection_create)
mcs_iov_backend_bin (collection_delete)
mcs_iov_backend_bin (export_shared)
mcs_iov_backend_bin (import_shared)
mcs_iov_backend_bin (list_collections)
mcs_iov_backend_bin (list_storages)
mcs_iov_backend_bin (locations)
mcs_iov_backend_bin (make_parameter)
mcs_iov_backend_bin (provider)
mcs_iov_backend_bin (range)
mcs_iov_backend_bin (state)
mcs_iov_backend_bin (storage_provider PRIVATE mcs_util_syscall)

# todo: how to ever remove an earlier persisted files-segment?

add_subdirectory (iov)
