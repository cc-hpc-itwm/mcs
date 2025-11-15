# Copyright (C) 2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

# \todo find_package (iov)
set (MCS_LIBIOV_REPO
  "git@gitlab-ce.gwdg.de:mcse-project/libioverbs.git"
  CACHE
  STRING
  "Location of the LibIOVerbs repository"
)
set (MCS_LIBIOV_REVISION
  "25d37d55aac2d6636342e1124ff4b9d4d241c5b6"
  CACHE
  STRING
  "git tag for the desired libIOVerbs revision"
)
include (FetchContent)

FetchContent_Declare (libioverbs
  GIT_REPOSITORY ${MCS_LIBIOV_REPO}
  GIT_TAG        ${MCS_LIBIOV_REVISION}
  SOURCE_DIR     ${CMAKE_BINARY_DIR}/3rdparty/libiov
  GIT_SHALLOW    TRUE
  SYSTEM
)
set (IOV_WITH_WERROR OFF)
set (IOV_BUILD_DOCS OFF)
set (IOV_BUILD_TESTING OFF)
set (IOV_BUILD_EXAMPLES OFF)
set (IOV_INSTALL OFF)
set (IOV_META_BACKEND "etcd")
set (IOV_META_BACKEND_CACHING "none")
set (IOV_BUILD_POSIX_PRELOADING OFF)
set (IOV_BUILD_FUSE_CLIENT OFF)
set (IOV_USE_TL_EXPECTED OFF)
set (IOV_USE_SYSTEM_TL_EXPECTED OFF)
set (IOV_USE_SYSTEM_FMT OFF)
set (IOV_TESTING_BACKEND "dummy")
FetchContent_MakeAvailable (libioverbs)
