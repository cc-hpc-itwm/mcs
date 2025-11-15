# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

add_library (mcs_iov_backend)
add_library (mcs_iov_backend_shared SHARED)

function (mcs_iov_backend_library _name)
  target_include_directories (${_name} PUBLIC include)

  target_link_libraries (${_name}
    PUBLIC mcs_config
    PUBLIC mcs_core
    PUBLIC iov::iov
    PUBLIC mcs_rpc
    PUBLIC mcs_serialization
    PUBLIC mcs_util_hash
    PUBLIC mcs_util_tuplish
    PUBLIC mcs_util
    PUBLIC mcs_util_ASIO
    PUBLIC mcs_util_FMT
  )
endfunction()

mcs_iov_backend_library (mcs_iov_backend)
mcs_iov_backend_library (mcs_iov_backend_shared)

set_target_properties (mcs_iov_backend_shared
  PROPERTIES OUTPUT_NAME mcs_iov_backend
)
set_target_properties (mcs_iov_backend_shared
  PROPERTIES POSITION_INDEPENDENT_CODE ON
)
if (MCS_INSTALL)
  install (TARGETS mcs_iov_backend LIBRARY)
  install (TARGETS mcs_iov_backend_shared LIBRARY)
endif()

add_subdirectory (bin)
add_subdirectory (src)
