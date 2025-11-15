# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

function (mcs_iov_backend_sources _name)
  target_sources (${_name}
    PRIVATE IOV/Backend.cpp
    PRIVATE IOV/Database.cpp
    PRIVATE IOV/UUID.cpp
    PRIVATE Location.cpp
    PRIVATE Locations.cpp
    PRIVATE Parameter.cpp
    PRIVATE Storage.cpp
    PRIVATE UsedStorage.cpp
    PRIVATE UsedStorages.cpp
    PRIVATE collection/ID.cpp
    PRIVATE detail/Bitmap.cpp
    PRIVATE detail/SpanDeleter.cpp
    PRIVATE iov_backend_ops.cpp
    PRIVATE provider/State.cpp
    PRIVATE provider/StoragesClients.cpp
    PRIVATE command/Locations.cpp
    PRIVATE command/Range.cpp
    PRIVATE command/collection/Append.cpp
    PRIVATE command/collection/Create.cpp
    PRIVATE command/collection/Delete.cpp
    PRIVATE command/storage/Add.cpp
    PRIVATE provider/distribution/AsEqualAsPossible.cpp
    PRIVATE storage/ID.cpp
  )
  target_link_libraries (${_name}
    PRIVATE mcs_config
    PRIVATE mcs_nonstd_scope
    PRIVATE mcs_serialization
    PRIVATE mcs_util
    PRIVATE mcs_util_read
  )
endfunction()

mcs_iov_backend_sources (mcs_iov_backend)
mcs_iov_backend_sources (mcs_iov_backend_shared)
