# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_iov_backend_bin_iov_workspace_name workspace_name.cpp)
target_link_libraries (mcs_iov_backend_bin_iov_workspace_name iov::iov)

function (mcs_iov_backend_bin_iov name)
  add_executable (mcs_iov_backend_iov_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_iov_backend_iov_${name}
    PRIVATE mcs_iov_backend_bin_iov_workspace_name
    PRIVATE fmt
    PRIVATE mcs_iov_backend
    PRIVATE mcs_rpc
    PRIVATE mcs_util
    PRIVATE mcs_util_read
    ${ARGN}
  )
  if (MCS_INSTALL)
    install (TARGETS mcs_iov_backend_iov_${name} RUNTIME)
  endif()
endfunction()

mcs_iov_backend_bin_iov (add_entry_to_database)
mcs_iov_backend_bin_iov (cat)
mcs_iov_backend_bin_iov (collection_create)
mcs_iov_backend_bin_iov (collection_delete)
mcs_iov_backend_bin_iov (collection_exists)
mcs_iov_backend_bin_iov (collection_uuid)
mcs_iov_backend_bin_iov (export PRIVATE mcs_util_syscall)
mcs_iov_backend_bin_iov (import PRIVATE mcs_util_syscall)
mcs_iov_backend_bin_iov (make_empty_database)
mcs_iov_backend_bin_iov (print_database)
mcs_iov_backend_bin_iov (remove_entry_from_database)
mcs_iov_backend_bin_iov (write)

# todo: batch creation of collections -> so far only possible in mcs
# as there is no agreed notion of "batch"

# scenario:
# - setup: n storage-machines, each provides m many bytes
# - import file into iov-space
# - use/modify data in iov-space
# - optional: shutdown/restart backend provider (and keep storages alive)
# - optional: if storage is persistent: shutdown, restart
# - use/modify data in iov-space
# - export from iov-space into file
