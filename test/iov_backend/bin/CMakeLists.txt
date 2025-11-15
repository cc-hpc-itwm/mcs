# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

set (_BIN_DIR "${CMAKE_BINARY_DIR}/iov_backend/bin")
set (TESTING_BASH_DIRECTORY "${PROJECT_SOURCE_DIR}/testing/bash")

set (TEST_IOV_BACKEND_COLLECTION_CREATE_BIN "${_BIN_DIR}/mcs_iov_backend_collection_create")
set (TEST_IOV_BACKEND_EXPORT_SHARED_BIN     "${_BIN_DIR}/mcs_iov_backend_export_shared")
set (TEST_IOV_BACKEND_IMPORT_SHARED_BIN     "${_BIN_DIR}/mcs_iov_backend_import_shared")
set (TEST_IOV_BACKEND_MAKE_PARAMETER_BIN    "${_BIN_DIR}/mcs_iov_backend_make_parameter")
set (TEST_IOV_BACKEND_PROVIDER_BIN          "${_BIN_DIR}/mcs_iov_backend_provider")
set (TEST_IOV_BACKEND_STORAGE_PROVIDER_BIN  "${_BIN_DIR}/mcs_iov_backend_storage_provider")

set (TEST_IOV_BACKEND_IOV_CAT_BIN                 "${_BIN_DIR}/iov/mcs_iov_backend_iov_cat")
set (TEST_IOV_BACKEND_IOV_COLLECTION_CREATE_BIN   "${_BIN_DIR}/iov/mcs_iov_backend_iov_collection_create")
set (TEST_IOV_BACKEND_IOV_COLLECTION_DELETE_BIN   "${_BIN_DIR}/iov/mcs_iov_backend_iov_collection_delete")
set (TEST_IOV_BACKEND_IOV_EXPORT_BIN              "${_BIN_DIR}/iov/mcs_iov_backend_iov_export")
set (TEST_IOV_BACKEND_IOV_IMPORT_BIN              "${_BIN_DIR}/iov/mcs_iov_backend_iov_import")
set (TEST_IOV_BACKEND_IOV_MAKE_EMPTY_DATABASE_BIN "${_BIN_DIR}/iov/mcs_iov_backend_iov_make_empty_database")
set (TEST_IOV_BACKEND_IOV_WRITE_BIN               "${_BIN_DIR}/iov/mcs_iov_backend_iov_write")

function (mcs_test_iov_backend_bin name)
  configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/${name}.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/${name}.sh
    FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
    @ONLY
  )

  add_test (
    NAME iov_backend_bin_${name}
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${name}.sh"
  )
endfunction()

mcs_test_iov_backend_bin (import_export_shared)
mcs_test_iov_backend_bin (iov_import_export)
mcs_test_iov_backend_bin (iov_write_cat)
