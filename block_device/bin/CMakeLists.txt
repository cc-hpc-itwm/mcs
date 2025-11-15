# Copyright (C) 2023-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_block_device_bin name)
  add_executable (mcs_block_device_bin_${name}
    ${name}.cpp
  )
  target_link_libraries (mcs_block_device_bin_${name}
    PRIVATE mcs_config
    PRIVATE fmt
    PRIVATE mcs_block_device
    PRIVATE mcs_rpc
    PRIVATE mcs_util
    PRIVATE mcs_util_read
    ${ARGN}
  )

  if (MCS_INSTALL)
    install (TARGETS mcs_block_device_bin_${name} RUNTIME)
  endif()
endfunction()

mcs_block_device_bin (block_size)
mcs_block_device_bin (blocks)
mcs_block_device_bin (cat)
mcs_block_device_bin (meta_data_provider PRIVATE mcs_util_syscall)
mcs_block_device_bin (number_of_blocks)
mcs_block_device_bin (remove)
mcs_block_device_bin (storage_provider PRIVATE mcs_util_syscall)
mcs_block_device_bin (write)
